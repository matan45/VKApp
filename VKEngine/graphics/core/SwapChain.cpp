#include "SwapChain.hpp"
#include "Device.hpp"
#include "Utilities.hpp"
#include "log/Logger.hpp"

namespace core {

	SwapChain::SwapChain(core::Device& device) :device{ device }
	{

	}

	void SwapChain::init(uint32_t width, uint32_t height)
	{
		createSwapchain(width, height);
		createImageViews();
		createDepthStencil();
	}

	void SwapChain::cleanUp()
	{
		device.getLogicalDevice().waitIdle();

		// Destroy swapchain image views first
		for (auto imageView : swapchainImageViews) {
			device.getLogicalDevice().destroyImageView(imageView);
		}
		swapchainImageViews.clear();

		device.getLogicalDevice().destroyImageView(swapchainDepthStencil.depthStencilView);
		device.getLogicalDevice().destroyImage(swapchainDepthStencil.depthStencilImage);
		device.getLogicalDevice().freeMemory(swapchainDepthStencil.depthStencilMemory);

		// Destroy swapchain
		if (swapchain) {
			swapchain.reset();  // vk::UniqueSwapchainKHR automatically cleans up
		}

	}

	void SwapChain::recreate(uint32_t width, uint32_t height)
	{
		cleanUp();
		init(width, height);
	}

	void SwapChain::createSwapchain(uint32_t width, uint32_t height)
	{
		SwapchainSupportDetails swapchainSupport = Utilities::querySwapchainSupport(device.getPhysicalDevice(), device.getSurface());

		vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.formats);
		vk::PresentModeKHR presentMode = choosePresentMode(swapchainSupport.presentModes);
		vk::Extent2D extent = chooseSwapExtent(width, height, swapchainSupport.capabilities);

		uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
		if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
			imageCount = swapchainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo{};
		createInfo.surface = device.getSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;  // For VR, this could be 2
		createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

		QueueFamilyIndices indices = Utilities::findQueueFamiliesFromDevice(device.getPhysicalDevice(), device.getSurface());
		std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsAndComputeFamily != indices.presentFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;  // We don't care about pixels obscured by other windows
		createInfo.oldSwapchain = nullptr;  // Use this for recreating the swapchain

		swapchain = device.getLogicalDevice().createSwapchainKHRUnique(createInfo);

		swapchainImages = device.getLogicalDevice().getSwapchainImagesKHR(swapchain.get());
		swapchainImageFormat = surfaceFormat.format;
		swapchainExtent = extent;
	}

	void SwapChain::createImageViews()
	{
		swapchainImageViews.resize(swapchainImages.size());

		for (size_t i = 0; i < swapchainImages.size(); i++) {
			using enum vk::ComponentSwizzle;

			vk::ImageViewCreateInfo createInfo{};
			createInfo.image = swapchainImages[i];
			createInfo.viewType = vk::ImageViewType::e2D;
			createInfo.format = swapchainImageFormat;
			createInfo.components.r = eIdentity;
			createInfo.components.g = eIdentity;
			createInfo.components.b = eIdentity;
			createInfo.components.a = eIdentity;

			createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			swapchainImageViews[i] = device.getLogicalDevice().createImageView(createInfo);
		}
	}

	void SwapChain::createDepthStencil()
	{
		swapchainDepthStencilFormat = findDepthStencilFormat();

		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = swapchainExtent.width;
		imageInfo.extent.height = swapchainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = swapchainDepthStencilFormat;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

		try {
			swapchainDepthStencil.depthStencilImage = device.getLogicalDevice().createImage(imageInfo);
		}
		catch (vk::SystemError& err) {
			loggerError("Failed to create depth stencil image: {}", err.what());
			throw;
		}

		vk::MemoryRequirements memRequirements = device.getLogicalDevice().getImageMemoryRequirements(swapchainDepthStencil.depthStencilImage);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Utilities::findMemoryType(device.getPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		try {
			swapchainDepthStencil.depthStencilMemory = device.getLogicalDevice().allocateMemory(allocInfo);
			device.getLogicalDevice().bindImageMemory(swapchainDepthStencil.depthStencilImage, swapchainDepthStencil.depthStencilMemory, 0);
		}
		catch (vk::SystemError& err) {
			loggerError("Failed to allocate depth stencil memory: {}", err.what());
			throw;
		}

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = swapchainDepthStencil.depthStencilImage;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = swapchainDepthStencilFormat;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (swapchainDepthStencilFormat >= vk::Format::eD16UnormS8Uint) {
			viewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}

		try {
			swapchainDepthStencil.depthStencilView = device.getLogicalDevice().createImageView(viewInfo);
		}
		catch (vk::SystemError& err) {
			loggerError("Failed to create depth stencil image view: {}", err.what());
			throw;
		}
	}

	vk::Format SwapChain::findDepthStencilFormat() const
	{
		using enum vk::Format;
		std::array<vk::Format, 5> candidates = {
			eD32SfloatS8Uint,
			eD32Sfloat,
			eD24UnormS8Uint,
			eD16UnormS8Uint,
			eD16Unorm
		};

		for (vk::Format format : candidates) {
			vk::FormatProperties props = device.getPhysicalDevice().getFormatProperties(format);
			if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
				return format;
			}
		}

		loggerError("Failed to find a supported format for depth stencil attachment!");
		throw std::runtime_error("Failed to find a supported format for depth stencil attachment");
	}

	vk::SurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
	{
		for (const auto& format : availableFormats) {
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return format;
			}
		}
		return availableFormats[0];  // Fallback to the first available format
	}

	vk::PresentModeKHR SwapChain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)const {
		for (const auto& presentMode : availablePresentModes) {
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				return presentMode;
			}
		}
		return vk::PresentModeKHR::eFifo;  // Fallback to guaranteed vsync
	}


	vk::Extent2D SwapChain::chooseSwapExtent(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& capabilities) const {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			// Fallback to window dimensions
			vk::Extent2D actualExtent = { 800, 600 };  // Default values, replace with window dimensions
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
			return actualExtent;
		}

	}

}
