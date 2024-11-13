#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../core/OffScreen.hpp"

namespace core {
	class Device;
	class SwapChain;
}


namespace render {

	class ClearColor
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;

		vk::RenderPass renderPass;
		std::vector<vk::Framebuffer> framebuffers;

		core::OffscreenResources& offscreenResources;

	public:
		explicit ClearColor(core::Device& device, core::SwapChain& swapChain, core::OffscreenResources& offscreenResources);
		~ClearColor() = default;

		void init();
		void recreate();
		void cleanUp() const;

		void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

	private:
		void createFrameBuffers();
		void createRenderPass();
	};

}

