#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
	class Device;
	class SwapChain;

	class CommandPool {
	private:
		Device& device;
		SwapChain& swapChain;

		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;

	public:
		explicit CommandPool(Device& device, SwapChain& swapChain);
		~CommandPool() = default;

		vk::CommandPool getCommandPool() const { return commandPool; }

		void cleanUp() const;

		vk::CommandBuffer getCommandBuffer(uint32_t index) const { return commandBuffers[index]; }

	private:
		void createCommandPool();
		void allocateCommandBuffers();
	};
}

