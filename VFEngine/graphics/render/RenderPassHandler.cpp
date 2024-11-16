#include "RenderPassHandler.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "ClearColor.hpp"
#include "IBL.hpp"

namespace render {
	RenderPassHandler::RenderPassHandler(core::Device& device, core::SwapChain& swapChain, core::OffscreenResources& offscreenResources) : device{ device },
		swapChain{ swapChain }, offscreenResources{ offscreenResources }
	{
		clearColor = new ClearColor(device, swapChain, offscreenResources);
		iblRenderer = new IBL(device, swapChain, offscreenResources);
	}

	RenderPassHandler::~RenderPassHandler()
	{
		delete clearColor;
		delete iblRenderer;
	}

	void RenderPassHandler::init()
	{
		clearColor->init();
	}

	void RenderPassHandler::recreate() const
	{
		iblRenderer->recreate();
		clearColor->recreate();
	}

	void RenderPassHandler::cleanUp() const
	{
		iblRenderer->cleanUp();
		clearColor->cleanUp();
	}

	void RenderPassHandler::draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		clearColor->recordCommandBuffer(commandBuffer, imageIndex);
		iblRenderer->recordCommandBuffer(commandBuffer, imageIndex);
	}

}
