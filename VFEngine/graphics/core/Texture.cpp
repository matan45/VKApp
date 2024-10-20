#include "Texture.hpp"
#include "Device.hpp"

namespace core {
	Texture::Texture(Device& device) :device{ device }
	{
		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
		poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().graphicsAndComputeFamily.value(); 

		commandPool = device.getLogicalDevice().createCommandPoolUnique(poolInfo);
	}

	Texture::~Texture()
	{
		device.getLogicalDevice().destroyBuffer(stagingBuffer);
		device.getLogicalDevice().freeMemory(stagingBufferMemory);
		device.getLogicalDevice().destroyImageView(imageView);
		device.getLogicalDevice().destroyImage(image);
		device.getLogicalDevice().freeMemory(imageMemory);
		device.getLogicalDevice().destroySampler(sampler);
	}

	void Texture::loadFromFile(std::string_view filePath)
	{
	}
}
