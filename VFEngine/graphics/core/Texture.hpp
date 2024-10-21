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

		vk::Sampler sampler;

		vk::UniqueDescriptorSet descriptorSet;
		vk::UniqueDescriptorSetLayout descriptorSetLayout;
		vk::UniqueDescriptorPool descriptorPool;

	public:
		explicit Texture(Device& device);
		~Texture();

		void loadFromFile(std::string_view filePath);
		const vk::DescriptorSet& getDescriptorSet() const { return descriptorSet.get(); }

	private:
		void createSampler();
		void copyBufferToImage(vk::Buffer buffer, uint32_t width, uint32_t height);

		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSet();
	};
}


