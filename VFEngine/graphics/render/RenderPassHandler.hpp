#pragma once
#include "../imguiPass/OffScreen.hpp"

namespace core {
	class Device;
	class SwapChain;
}


namespace render {

	class TriangleRenderer;

	class RenderPassHandler
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;

		TriangleRenderer* triangleRenderer{ nullptr };

		std::vector<imguiPass::OffscreenResources>& offscreenResources;
	public:
		explicit RenderPassHandler(core::Device& device, core::SwapChain& swapChain, std::vector<imguiPass::OffscreenResources>& offscreenResources);
		~RenderPassHandler();

		void init();

		void recreate() const;


		void cleanUp() const;

		void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

	};
}

