#pragma once
#include <vulkan/vulkan.hpp>
#include <string>

namespace core {
	class Device;

	class Texture
	{
	private:
		Device& device;
		vk::UniqueCommandPool commandPool;

		vk::Image image;
		vk::DeviceMemory imageMemory;
		vk::ImageView imageView;

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		vk::Sampler sampler;
		vk::DescriptorSet descriptorSet;

	public:
		explicit Texture(Device& device);
		~Texture();

		void loadFromFile(std::string_view filePath);
		const vk::DescriptorSet& getDescriptorSet() const { return descriptorSet; }

	private:
	};
}


