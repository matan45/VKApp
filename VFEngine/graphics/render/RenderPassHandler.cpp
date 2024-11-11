#include "RenderPassHandler.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "TriangleRenderer.hpp"
#include "IBL.hpp"

namespace render {
	RenderPassHandler::RenderPassHandler(core::Device& device, core::SwapChain& swapChain, core::OffscreenResources& offscreenResources) : device{ device },
		swapChain{ swapChain }, offscreenResources{ offscreenResources }
	{
		triangleRenderer = new TriangleRenderer(device, swapChain, offscreenResources);
		iblRenderer = new IBL(device, swapChain, offscreenResources);
	}

	RenderPassHandler::~RenderPassHandler()
	{
		delete triangleRenderer;
		delete iblRenderer;
	}

	void RenderPassHandler::init()
	{
		triangleRenderer->init();
		iblRenderer->init("../../resources/shaders/ibl/Arches_E_PineTree_3k.hdr");
	}

	void RenderPassHandler::recreate() const
	{
		iblRenderer->recreate();
		triangleRenderer->recreate();
	}

	void RenderPassHandler::cleanUp() const
	{
		iblRenderer->cleanUp();
		triangleRenderer->cleanUp();
	}

	void RenderPassHandler::draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		iblRenderer->recordCommandBuffer(commandBuffer, imageIndex);
		triangleRenderer->recordCommandBuffer(commandBuffer, imageIndex);
	}

}
