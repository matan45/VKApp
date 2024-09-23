#pragma once
#include <vulkan/vulkan.hpp>

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

	public:
		explicit RenderPassHandler(core::Device& device, core::SwapChain& swapChain);
		~RenderPassHandler();

		void init();

		void recreate(uint32_t width, uint32_t height) const;


		void cleanUp() const;

		void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

	};
}

