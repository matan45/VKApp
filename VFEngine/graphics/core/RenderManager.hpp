#pragma once
#include <vulkan/vulkan.hpp>

namespace window {
	class Window;
}

namespace render {
	class RenderPassHandler;
}

namespace imguiPass {
	class ImguiRender;
}

namespace core {
	class Device;
	class SwapChain;
	class CommandPool;

	class RenderManager
	{
	private:
		Device& device;
		SwapChain& swapChain;
		window::Window& window;
		CommandPool* commandPool{ nullptr };
		//render::RenderPassHandler* renderPassHandler{ nullptr };
		imguiPass::ImguiRender* imguiRender{ nullptr };

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;
		vk::Fence renderFence;

	public:
		explicit RenderManager(Device& device, SwapChain& swapChain, window::Window& window);
		~RenderManager();

		void init();

		void render();

		void recreate(uint32_t width, uint32_t height) const;

		inline static uint32_t imageIndex;

		void cleanUp() const;

	private:
		void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

		void present(uint32_t imageIndex) const;
	};
}


