#include "RenderManager.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"
#include "CommandPool.hpp"
#include "log/Logger.hpp"
#include "../window/Window.hpp"
#include "../imguiPass/ImguiRender.hpp"

namespace core {


	RenderManager::RenderManager(Device& device, SwapChain& swapChain, window::Window& window) : device{ device },
		swapChain{ swapChain }, window{ window }
	{

	}

	RenderManager::~RenderManager()
	{
		delete commandPool;
		delete imguiRender;
	}

	void RenderManager::init()
	{
		commandPool = new CommandPool(device, swapChain);

		imguiRender = new imguiPass::ImguiRender(device, swapChain, *commandPool, window);
		imguiRender->init();

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
		vk::Result result = device.getLogicalDevice().acquireNextImageKHR(
			swapChain.getSwapchain(),
			UINT64_MAX,
			imageAvailableSemaphore,
			nullptr,
			&imageIndex
		);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreate(window.getHeight(), window.getWidth());  // Handle swapchain recreation
			return;
		}

		// Reset the fence before using it for synchronization
		result = device.getLogicalDevice().resetFences(1, &renderFence);
		if (result != vk::Result::eSuccess) {
			loggerError("failed to reset fences");
		}

		commandPool->resetCommandBuffer(imageIndex);

		vk::CommandBuffer commandBuffer = commandPool->getCommandBuffer(imageIndex);

		// Begin recording commands for the acquired image
		commandBuffer.begin(vk::CommandBufferBeginInfo{});

		const auto& image = swapChain.getSwapchainImage(imageIndex);

		// Render the scene using the command buffer for this swapchain image
		draw(commandBuffer, imageIndex);

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

	void RenderManager::recreate(uint32_t width, uint32_t height) const
	{
		if (width == 0 || height == 0) return;  // Skip if minimized
		commandPool->recreate();  // Reallocate command buffers if needed

		imguiRender->recreate(width, height);
	}

	void RenderManager::cleanUp() const
	{
		device.getLogicalDevice().waitIdle();

		commandPool->cleanUp();

		imguiRender->cleanUp();

		device.getLogicalDevice().destroySemaphore(imageAvailableSemaphore);
		device.getLogicalDevice().destroySemaphore(renderFinishedSemaphore);
		device.getLogicalDevice().destroyFence(renderFence);
	}

	void RenderManager::draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		//create a render pass class so we can use here and offscreen class
		//here for now only use imgui render
		imguiRender->render(commandBuffer, imageIndex);
	}

	void RenderManager::present(uint32_t imageIndex) const
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
			recreate(window.getHeight(), window.getWidth());
		}
	}

	void RenderManager::beginImageBarrier(const vk::CommandBuffer& commandBuffer, const vk::ImageSubresourceRange& subresourceRange, const vk::Image& image)const
	{

		vk::ImageMemoryBarrier beginImageMemoryBarrier{};
		beginImageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		beginImageMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		beginImageMemoryBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
		beginImageMemoryBarrier.image = image;
		beginImageMemoryBarrier.subresourceRange = subresourceRange;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlags(), nullptr, nullptr, beginImageMemoryBarrier);
	}

	void RenderManager::beginImageDepthStencilBarrier(const vk::CommandBuffer& commandBuffer, const vk::ImageSubresourceRange& subresourceRange, const vk::Image& image) const
	{
		vk::ImageMemoryBarrier beginImageDepthStencilMemoryBarrier{};
		beginImageDepthStencilMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		beginImageDepthStencilMemoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
		beginImageDepthStencilMemoryBarrier.newLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		beginImageDepthStencilMemoryBarrier.image = image;
		beginImageDepthStencilMemoryBarrier.subresourceRange = subresourceRange;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::DependencyFlags(), nullptr, nullptr, beginImageDepthStencilMemoryBarrier);
	}

	void RenderManager::endImageBarrier(const vk::CommandBuffer& commandBuffer, const vk::ImageSubresourceRange& subresourceRange, const vk::Image& image)const
	{
		vk::ImageMemoryBarrier endImageMemoryBarrier{};
		endImageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		endImageMemoryBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
		endImageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
		endImageMemoryBarrier.image = image;
		endImageMemoryBarrier.subresourceRange = subresourceRange;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlags(), nullptr, nullptr, endImageMemoryBarrier);
	}

	void RenderManager::endImageBarrierDepthStencilBarrier(const vk::CommandBuffer& commandBuffer, const vk::ImageSubresourceRange& subresourceRange, const vk::Image& image)const
	{
		vk::ImageMemoryBarrier endImageMemoryBarrier{};
		endImageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		endImageMemoryBarrier.oldLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		endImageMemoryBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
		endImageMemoryBarrier.image = image;
		endImageMemoryBarrier.subresourceRange = subresourceRange;

		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::DependencyFlags(), nullptr, nullptr, endImageMemoryBarrier);
	}

}

