#include "RenderPassHandler.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "TriangleRenderer.hpp"

namespace render {
	RenderPassHandler::RenderPassHandler(core::Device& device, core::SwapChain& swapChain, std::vector<imguiPass::OffscreenResources>& offscreenResources) : device{ device },
		swapChain{ swapChain }, offscreenResources{ offscreenResources }
	{
		triangleRenderer = new TriangleRenderer(device, swapChain, offscreenResources);
	}

	RenderPassHandler::~RenderPassHandler()
	{
		delete triangleRenderer;
	}

	void RenderPassHandler::init()
	{
		triangleRenderer->init();
	}

	void RenderPassHandler::recreate(uint32_t width, uint32_t height) const
	{
		triangleRenderer->recreate(width, height);
	}

	void RenderPassHandler::cleanUp() const
	{
		triangleRenderer->cleanUp();
	}

	void RenderPassHandler::draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		triangleRenderer->recordCommandBuffer(commandBuffer,imageIndex);
	}

}
