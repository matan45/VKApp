#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
	class Device;
	class SwapChain;
	class CommandPool;

	class RenderManager
	{
	private:
		Device& device;
		SwapChain& swapChain;
		CommandPool* commandPool;

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;
		vk::Fence renderFence;

	public:
		explicit RenderManager(Device& device, SwapChain& swapChain);
		~RenderManager();

		void init();

		void render();

		void recreate(uint32_t width, uint32_t height);


		void cleanUp() const;

	private:
		void draw(const vk::CommandBuffer& commandBuffer);

		void present(uint32_t imageIndex);

		void transitionImageLayout(
			vk::CommandBuffer commandBuffer,
			vk::Image image,
			vk::Format format,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout
		) const;

		void transitionDepthImageLayout(
			vk::CommandBuffer commandBuffer,
			vk::Image image,
			vk::Format format,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout
		)const;
	};
}


