
#include "Device.hpp"
#include "SwapChain.hpp"
#include "CommandPool.hpp"
#include "log/Logger.hpp"
#include "RenderManager.hpp"

namespace core {


	RenderManager::RenderManager(Device& device, SwapChain& swapChain) : device{ device },
		swapChain{ swapChain }
	{
		
	}

	RenderManager::~RenderManager()
	{
		delete commandPool;
	}

	void RenderManager::init()
	{
		commandPool = new CommandPool(device, swapChain);
		// Create semaphores for synchronization
		vk::SemaphoreCreateInfo semaphoreInfo{};

		imageAvailableSemaphore = device.getLogicalDevice().createSemaphore(semaphoreInfo);
		renderFinishedSemaphore = device.getLogicalDevice().createSemaphore(semaphoreInfo);

		// Create a fence for GPU-CPU synchronization
		vk::FenceCreateInfo fenceInfo{};
		renderFence = device.getLogicalDevice().createFence(fenceInfo);
	}

	void RenderManager::render()
	{
		// Acquire the next image from the swapchain
		uint32_t imageIndex;
		vk::Result result = device.getLogicalDevice().acquireNextImageKHR(
			swapChain.getSwapchain(),
			UINT64_MAX,
			imageAvailableSemaphore,
			nullptr,
			&imageIndex
		);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreate(800, 600);  // Handle swapchain recreation
			return;
		}

		// Reset the fence before using it for synchronization
		result = device.getLogicalDevice().resetFences(1, &renderFence);
		if (result != vk::Result::eSuccess) {
			loggerError("failed to reset fences");
		}

		// Begin recording commands for the acquired image
		commandPool->getCommandBuffer(imageIndex).begin(vk::CommandBufferBeginInfo{});

		// Transition the image to `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` for rendering
		transitionImageLayout(
			commandPool->getCommandBuffer(imageIndex),
			swapChain.getSwapchainImage(imageIndex),
			swapChain.getSwapchainImageFormat(),
			vk::ImageLayout::eUndefined,  // Could also be `ePresentSrcKHR`
			vk::ImageLayout::eColorAttachmentOptimal  // Ready for rendering
		);

		// Transition the depth image layout for rendering
		transitionDepthImageLayout(
			commandPool->getCommandBuffer(imageIndex),
			swapChain.getDepthStencilImage(),
			swapChain.getSwapchainDepthStencilFormat(),
			vk::ImageLayout::eUndefined,  // Start with undefined layout
			vk::ImageLayout::eDepthStencilAttachmentOptimal  // Ready for depth testing
		);

		// Render the scene using the command buffer for this swapchain image
		draw(commandPool->getCommandBuffer(imageIndex));

		// Transition the image to `VK_IMAGE_LAYOUT_PRESENT_SRC_KHR` for presentation
		transitionImageLayout(
			commandPool->getCommandBuffer(imageIndex),
			swapChain.getSwapchainImage(imageIndex),
			swapChain.getSwapchainImageFormat(),
			vk::ImageLayout::eColorAttachmentOptimal,  // After rendering
			vk::ImageLayout::ePresentSrcKHR  // Ready for presentation
		);

		// End command buffer recording
		commandPool->getCommandBuffer(imageIndex).end();

		// Submit the command buffer for rendering
		vk::SubmitInfo submitInfo{};
		std::array <vk::Semaphore, 1> waitSemaphores = { imageAvailableSemaphore };
		std::array < vk::Semaphore, 1> signalSemaphores = { renderFinishedSemaphore };
		std::array < vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		vk::CommandBuffer cmdBuffer = commandPool->getCommandBuffer(imageIndex);
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		device.getGraphicsQueue().submit(submitInfo, renderFence);

		// Wait for the GPU to finish executing the command buffer before continuing
		result = device.getLogicalDevice().waitForFences(1, &renderFence, VK_TRUE, UINT64_MAX);
		if (result != vk::Result::eSuccess) {
			loggerError("failed to wait fences");
		}

		// Present the rendered image
		present(imageIndex);

	}

	void RenderManager::recreate(uint32_t width, uint32_t height)
	{

	}

	void RenderManager::cleanUp() const
	{
		commandPool->cleanUp();

		device.getLogicalDevice().destroySemaphore(imageAvailableSemaphore);
		device.getLogicalDevice().destroySemaphore(renderFinishedSemaphore);
		device.getLogicalDevice().destroyFence(renderFence);
	}

	void RenderManager::draw(const vk::CommandBuffer& commandBuffer)
	{

	}

	void RenderManager::present(uint32_t imageIndex)
	{

		// Present the image to the screen
		vk::PresentInfoKHR presentInfo{};
		std::array<vk::Semaphore, 1> waitSemaphores = { renderFinishedSemaphore };
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		presentInfo.pWaitSemaphores = waitSemaphores.data();
		std::array <vk::SwapchainKHR, 1> swapChains = { swapChain.getSwapchain() };
		presentInfo.swapchainCount = static_cast<uint32_t>(swapChains.size());
		presentInfo.pSwapchains = swapChains.data();
		presentInfo.pImageIndices = &imageIndex;

		vk::Result result = device.getPresentQueue().presentKHR(&presentInfo);

		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
			recreate(800, 600);  // Handle swapchain recreation
		}
	}

	void RenderManager::transitionImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const
	{
		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		// Transition for color images
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
			barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal && newLayout == vk::ImageLayout::ePresentSrcKHR) {
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eNoneKHR;

			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		}
		else {
			throw std::invalid_argument("Unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(
			sourceStage, destinationStage,
			vk::DependencyFlags{},
			nullptr, nullptr, barrier
		);
	}

	void RenderManager::transitionDepthImageLayout(vk::CommandBuffer commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const
	{
		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;

		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
		if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
			barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
			barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else {
			throw std::invalid_argument("Unsupported layout transition for depth image!");
		}

		commandBuffer.pipelineBarrier(
			sourceStage, destinationStage,
			vk::DependencyFlags{},
			nullptr, nullptr, barrier
		);
	}

}

