#include "Utilities.hpp"
#include "print/Logger.hpp"

namespace core {

	QueueFamilyIndices Utilities::findQueueFamiliesFromDevice(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
	{
		QueueFamilyIndices indices;
		const std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		if (debug) {
			loggerInfo("Found {} queue families.", queueFamilies.size());
		}

		int i = 0;
		for (const vk::QueueFamilyProperties& queueFamily : queueFamilies) {
			if (debug) {
				loggerInfo("Queue Family {}: Graphics: {}, Compute: {}, Transfer: {}",
					i,
					(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) ? "Yes" : "No",
					(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) ? "Yes" : "No",
					(queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) ? "Yes" : "No"
				);
			}

			if (device.getSurfaceSupportKHR(i, surface)) {
				indices.presentFamily = i;
			}

			if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
				indices.graphicsAndComputeFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		if (!indices.isComplete()) {
			if (debug) {
				loggerWarning("Could not find complete queue family support.");
			}
		}

		return indices;
	}

	core::SwapchainSupportDetails Utilities::querySwapchainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
	{
		SwapchainSupportDetails details;
		details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		details.formats = device.getSurfaceFormatsKHR(surface);
		details.presentModes = device.getSurfacePresentModesKHR(surface);
		return details;
	}

	uint32_t Utilities::findMemoryType(const vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		loggerError("Failed to find suitable memory type.");
		return 0;
	}

	vk::UniqueCommandBuffer Utilities::beginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool) {
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = commandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		vk::UniqueCommandBuffer commandBuffer;
		try {
			commandBuffer = std::move(device.allocateCommandBuffersUnique(allocInfo).front());
		}
		catch (const vk::SystemError& err) {
			loggerError("Failed to allocate command buffer: {}", err.what());
			throw;
		}

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		commandBuffer->begin(beginInfo);

		return commandBuffer;
	}

	void Utilities::endSingleTimeCommands(const vk::Queue& queue, const vk::UniqueCommandBuffer& commandBuffer,const vk::Fence& renderFence)
	{
		commandBuffer->end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(*commandBuffer);

		try {
			queue.submit(submitInfo, renderFence);
			queue.waitIdle();
		}
		catch (const vk::SystemError& err) {
			loggerError("Failed to submit command buffer: {}", err.what());
		}
	}

	void Utilities::transitionImageLayout(const vk::CommandBuffer& commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask)
	{
		using enum vk::AccessFlagBits;
		using enum vk::ImageLayout;
		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == eUndefined && newLayout == eColorAttachmentOptimal) {
			barrier.srcAccessMask = eNoneKHR;
			barrier.dstAccessMask = eColorAttachmentWrite;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		}
		else if (oldLayout == eColorAttachmentOptimal && newLayout == eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = eColorAttachmentWrite;
			barrier.dstAccessMask = eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

		}
		else if (oldLayout == eUndefined && newLayout == eDepthStencilAttachmentOptimal) {
			barrier.srcAccessMask = eNoneKHR;
			barrier.dstAccessMask = eDepthStencilAttachmentWrite;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;

		}
		else if (oldLayout == eDepthStencilAttachmentOptimal && newLayout == eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = eDepthStencilAttachmentWrite;
			barrier.dstAccessMask = eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eLateFragmentTests;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == eUndefined && newLayout == eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = eNone;
			barrier.dstAccessMask = eShaderRead;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == eUndefined && newLayout == eTransferDstOptimal) {
			barrier.srcAccessMask = eNone;
			barrier.dstAccessMask = eTransferWrite;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == eTransferDstOptimal && newLayout == eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = eTransferWrite;
			barrier.dstAccessMask = eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}

		commandBuffer.pipelineBarrier(
			sourceStage, destinationStage,
			vk::DependencyFlags{},
			nullptr, nullptr, barrier
		);
	}

	void Utilities::createBuffer(const BufferInfoRequest& bufferInfo, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
	{
		vk::BufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.size = bufferInfo.size;
		bufferCreateInfo.usage = bufferInfo.usage;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

		buffer = bufferInfo.logicalDevice.createBuffer(bufferCreateInfo);

		vk::MemoryRequirements memRequirements = bufferInfo.logicalDevice.getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = core::Utilities::findMemoryType(
			bufferInfo.physicalDevice,
			memRequirements.memoryTypeBits,
			bufferInfo.properties
		);

		bufferMemory = bufferInfo.logicalDevice.allocateMemory(allocInfo);
		bufferInfo.logicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
	}

	void Utilities::createImage(const ImageInfoRequest& imageInfo, vk::Image& image, vk::DeviceMemory& imageMemory)
	{
		vk::ImageCreateInfo imageCreateInfo{};
		imageCreateInfo.imageType = vk::ImageType::e2D;
		imageCreateInfo.extent.width = imageInfo.width;
		imageCreateInfo.extent.height = imageInfo.height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = imageInfo.layers;
		imageCreateInfo.format = imageInfo.format;
		imageCreateInfo.tiling = imageInfo.tiling;
		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageCreateInfo.usage = imageInfo.usage;
		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		imageCreateInfo.flags = imageInfo.imageFlags;

		image = imageInfo.logicalDevice.createImage(imageCreateInfo);

		vk::MemoryRequirements memRequirements = imageInfo.logicalDevice.getImageMemoryRequirements(image);
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = core::Utilities::findMemoryType(imageInfo.physicalDevice, memRequirements.memoryTypeBits, imageInfo.properties);

		imageMemory = imageInfo.logicalDevice.allocateMemory(allocInfo);
		imageInfo.logicalDevice.bindImageMemory(image, imageMemory, 0);
	}

	void Utilities::createImageView(const ImageViewInfoRequest& imageInfoView, vk::ImageView& imageView)
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = imageInfoView.image;
		viewInfo.viewType = imageInfoView.imageType;
		viewInfo.format = imageInfoView.format;
		viewInfo.subresourceRange.aspectMask = imageInfoView.aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = imageInfoView.layerCount;

		imageView = imageInfoView.logicalDevice.createImageView(viewInfo);
	}

}

