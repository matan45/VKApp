#pragma once
#include <vulkan/vulkan.hpp>
#include <imgui.h>
#include <vector>

namespace core{
	class Device;
	class SwapChain;
	class CommandPool;
}
namespace window {
	class Window;
}

namespace imgui {
	class ImguiRender
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;
		core::CommandPool& commandPool;
		window::Window& window;

		vk::DescriptorPool imGuiDescriptorPool;
		vk::RenderPass imGuiRenderPass;
		std::vector<vk::Framebuffer> imGuiFrameBuffers;

	public:
		void init();
		explicit ImguiRender(core::Device& device, core::SwapChain& swapChain, core::CommandPool& commandPool, window::Window& window);
		~ImguiRender() = default;

		void cleanUp() const;
		void recreate(uint32_t width, uint32_t height);
		void render(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

	private:
		
		void theme() const;
		void createRenderPass();
		void createDescriptorPool();
		void createFrameBuffers();

	};
}


