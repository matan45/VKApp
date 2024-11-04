#pragma once
#include "../core/OffScreen.hpp"

namespace core {
	class Device;
	class SwapChain;
}


namespace render {

	class TriangleRenderer;
	class IBL;

	class RenderPassHandler
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;

		TriangleRenderer* triangleRenderer{ nullptr };
		IBL* iblRenderer{ nullptr };

		std::vector<core::OffscreenResources>& offscreenResources;
	public:
		explicit RenderPassHandler(core::Device& device, core::SwapChain& swapChain, std::vector<core::OffscreenResources>& offscreenResources);
		~RenderPassHandler();

		void init();

		void recreate() const;


		void cleanUp() const;

		void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

	};
}

