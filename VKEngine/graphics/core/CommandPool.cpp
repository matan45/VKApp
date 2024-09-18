#include "CommandPool.hpp"
#include "Device.hpp"
#include "SwapChain.hpp"

namespace core {

	CommandPool::CommandPool(Device& device, SwapChain& swapChain) : device{ device }, swapChain{ swapChain } {
		createCommandPool();
		allocateCommandBuffers();
	}

	void CommandPool::allocateCommandBuffers() {
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = getCommandPool();
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = swapChain.getImageCount();

		commandBuffers = device.getLogicalDevice().allocateCommandBuffers(allocInfo);
	}

	void CommandPool::createCommandPool() {
		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().graphicsAndComputeFamily.value();
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

		commandPool = device.getLogicalDevice().createCommandPool(poolInfo);
	}

	void CommandPool::cleanUp() const {

		device.getLogicalDevice().freeCommandBuffers(commandPool, commandBuffers);

		if (commandPool) {
			device.getLogicalDevice().destroyCommandPool(commandPool);
		}
	}

}
