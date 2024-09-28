#include "OffScreenViewPort.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/CommandPool.hpp"
#include "../core/Utilities.hpp"
#include "../core/RenderManager.hpp"
#include "../render/RenderPassHandler.hpp"
#include "log/Logger.hpp"

#include <imgui_impl_vulkan.h>


namespace imguiPass {
	OffScreenViewPort::OffScreenViewPort(core::Device& device, core::SwapChain& swapChain) :device{ device }
		, swapChain{ swapChain }
	{
		commandPool = new core::CommandPool(device, swapChain);
	}

	OffScreenViewPort::~OffScreenViewPort()
	{
		delete commandPool;
		delete renderPassHandler;  // Properly clean up the render pass handler
	}

	void OffScreenViewPort::init()
	{
		createSampler();
		createOffscreenResources();

		renderPassHandler = new render::RenderPassHandler(device, swapChain, offscreenResources);
		renderPassHandler->init();
	}

	vk::DescriptorSet OffScreenViewPort::render()
	{

		// Get the command buffer for this frame
		vk::CommandBuffer commandBuffer = commandPool->getCommandBuffer(core::RenderManager::imageIndex);

		// Reset the command buffer for reuse
		commandBuffer.reset();

		// Begin recording commands for the acquired image
		commandBuffer.begin(vk::CommandBufferBeginInfo{});

		// Begin the render pass (record drawing commands here)
		draw(commandBuffer, core::RenderManager::imageIndex);

		// End command buffer recording
		commandBuffer.end();

		// Submit the command buffer to the compute queue
		vk::SubmitInfo submitInfo(
			0, nullptr, nullptr,
			1, &commandBuffer,
			0, nullptr
		);
		device.getGraphicsQueue().submit(submitInfo, nullptr);
		device.getGraphicsQueue().waitIdle();

		// Return the descriptor set for ImGui rendering
		return offscreenResources[core::RenderManager::imageIndex].descriptorSet;
	}

	void OffScreenViewPort::cleanUp() const
	{
		device.getLogicalDevice().waitIdle();

		renderPassHandler->cleanUp();

		commandPool->cleanUp();

		device.getLogicalDevice().destroySampler(sampler);

		for (auto const& resources : offscreenResources) {
			device.getLogicalDevice().destroyImageView(resources.colorImageView);
			device.getLogicalDevice().destroyImageView(resources.depthImageView);
			device.getLogicalDevice().destroyImage(resources.colorImage);
			device.getLogicalDevice().destroyImage(resources.depthImage);
			device.getLogicalDevice().freeMemory(resources.colorImageMemory);
			device.getLogicalDevice().freeMemory(resources.depthImageMemory);
		}
	}

	void OffScreenViewPort::draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		renderPassHandler->draw(commandBuffer, imageIndex);
	}

	void OffScreenViewPort::createOffscreenResources()
	{
		vk::Format colorFormat = swapChain.getSwapchainImageFormat();
		vk::Format depthFormat = swapChain.getSwapchainDepthStencilFormat();

		for (size_t i = 0; i < swapChain.getImageCount(); i++) {
			OffscreenResources resources;

			// Create color image for off-screen rendering
			createImage(colorFormat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, resources.colorImage, resources.colorImageMemory);
			createImageView(resources.colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, resources.colorImageView);

			// Create depth image
			createImage(depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment, resources.depthImage, resources.depthImageMemory);
			createImageView(resources.depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, resources.depthImageView);

			// Create framebuffer
			updateDescriptorSets(resources.descriptorSet, resources.colorImageView);

			// Store resources
			offscreenResources.push_back(std::move(resources));
		}
	}

	void OffScreenViewPort::createImage(vk::Format format, vk::ImageUsageFlags usage, vk::Image& image, vk::DeviceMemory& deviceMemory) const
	{
		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = swapChain.getSwapchainExtent().width;
		imageInfo.extent.height = swapChain.getSwapchainExtent().height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		image = device.getLogicalDevice().createImage(imageInfo);

		vk::MemoryRequirements memRequirements = device.getLogicalDevice().getImageMemoryRequirements(image);
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = core::Utilities::findMemoryType(device.getPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		deviceMemory = device.getLogicalDevice().allocateMemory(allocInfo);
		device.getLogicalDevice().bindImageMemory(image, deviceMemory, 0);
	}

	void OffScreenViewPort::createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView) const
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		imageView = device.getLogicalDevice().createImageView(viewInfo);
	}

	void OffScreenViewPort::updateDescriptorSets(vk::DescriptorSet& descriptorSet, const vk::ImageView& imageView) const
	{
		descriptorSet = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void OffScreenViewPort::createSampler()
	{
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;

		vk::PhysicalDeviceProperties properties = device.getPhysicalDevice().getProperties();

		// Retrieve the maximum anisotropy level supported by the device
		float maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		// Mipmapping options
		samplerInfo.anisotropyEnable = VK_TRUE;  // Enable anisotropic filtering if supported
		samplerInfo.maxAnisotropy = maxAnisotropy;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;  // Use black for border sampling
		samplerInfo.unnormalizedCoordinates = VK_FALSE;  // Use normalized coordinates [0, 1]
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = vk::CompareOp::eAlways;


		// Create the sampler
		sampler = device.getLogicalDevice().createSampler(samplerInfo);
	}

}
