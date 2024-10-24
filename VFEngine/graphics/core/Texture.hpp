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

		vk::DescriptorSet descriptorSet;
		vk::UniqueDescriptorPool descriptorPool;

	public:
		explicit Texture(Device& device);
		~Texture();

		void loadFromFile(std::string_view filePath, vk::Format format = vk::Format::eR8G8B8A8Srgb, bool isEditor = true);
		const vk::DescriptorSet& getDescriptorSet() const { return descriptorSet; }
		const vk::ImageView& getImageView() const { return imageView; }
		const vk::Sampler& getSampler() const { return sampler; }

	private:
		void createSampler();
		void copyBufferToImage(vk::Buffer buffer, uint32_t width, uint32_t height);
	};
}


