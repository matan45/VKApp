#include "SwapChain.hpp"
#include "Device.hpp"
#include "Utilities.hpp"

namespace core {

	SwapChain::SwapChain(core::Device& device) :device{ device }
	{

	}

	void SwapChain::init()
	{
		createSwapchain();
		createImageViews();
	}

	void SwapChain::cleanUp()
	{
		// Destroy swapchain image views first
		for (auto imageView : swapchainImageViews) {
			device.getLogicalDevice().destroyImageView(imageView);
		}
		swapchainImageViews.clear();

		// Destroy swapchain
		if (swapchain) {
			swapchain.reset();  // vk::UniqueSwapchainKHR automatically cleans up
		}
	}

	void SwapChain::recreate()
	{
		cleanUp();
		init();
	}

	void SwapChain::createSwapchain()
	{
		SwapchainSupportDetails swapchainSupport = Utilities::querySwapchainSupport(device.getPhysicalDevice(), device.getSurface());

		vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.formats);
		vk::PresentModeKHR presentMode = choosePresentMode(swapchainSupport.presentModes);
		vk::Extent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

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


	vk::Extent2D SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const {
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
