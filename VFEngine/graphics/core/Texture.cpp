#include "Texture.hpp"
#include "Device.hpp"
#include "resource/TextureResource.hpp"
#include "print/Logger.hpp"
#include <imgui_impl_vulkan.h>

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
		device.getLogicalDevice().waitIdle();
		device.getLogicalDevice().destroyImageView(imageView);
		device.getLogicalDevice().destroyImage(image);
		device.getLogicalDevice().freeMemory(imageMemory);
		device.getLogicalDevice().destroySampler(sampler);
	}

	void Texture::loadFromFile(std::string_view filePath, bool isEditor)
	{
		resource::TextureData textureData = resource::TextureResource::loadTexture(filePath);
		vk::DeviceSize imageSize = textureData.width * textureData.height * 4;

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingBufferMemory;

		BufferInfo bufferInfo(device.getLogicalDevice(), device.getPhysicalDevice());
		bufferInfo.size = imageSize;
		bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		bufferInfo.properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		Utilities::createBuffer(bufferInfo, stagingBuffer, stagingBufferMemory);

		void* data;
		if (vk::Result result = device.getLogicalDevice().mapMemory(stagingBufferMemory, 0, imageSize, {}, &data); result != vk::Result::eSuccess) {
			loggerError("failed to map memory");
		}
		memcpy(data, textureData.textureData.data(), imageSize);
		device.getLogicalDevice().unmapMemory(stagingBufferMemory);


		ImageInfo imageInfo(device.getLogicalDevice(), device.getPhysicalDevice());
		imageInfo.width = textureData.width;
		imageInfo.height = textureData.height;
		imageInfo.format = vk::Format::eR8G8B8A8Srgb;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		imageInfo.properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
		Utilities::createImage(imageInfo, image, imageMemory);

		vk::UniqueCommandBuffer commandTransitionA = core::Utilities::beginSingleTimeCommands(device.getLogicalDevice(), commandPool.get());
		Utilities::transitionImageLayout(commandTransitionA.get(), image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, vk::ImageAspectFlagBits::eColor);
		Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandTransitionA);

		copyBufferToImage(stagingBuffer, textureData.width, textureData.height);

		vk::UniqueCommandBuffer commandTransitionB = core::Utilities::beginSingleTimeCommands(device.getLogicalDevice(), commandPool.get());
		Utilities::transitionImageLayout(commandTransitionB.get(), image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
		Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandTransitionB);


		device.getLogicalDevice().destroyBuffer(stagingBuffer);
		device.getLogicalDevice().freeMemory(stagingBufferMemory);

		createSampler();
		Utilities::createImageView(device.getLogicalDevice(), image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, imageView);
		if (isEditor) {
			descriptorSet = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else {
			createDescriptorPool();
			createDescriptorSetLayout();
			createDescriptorSet();
		}
	}

	void Texture::createSampler()
	{
		using enum vk::SamplerAddressMode;
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = eRepeat;
		samplerInfo.addressModeV = eRepeat;
		samplerInfo.addressModeW = eRepeat;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		sampler = device.getLogicalDevice().createSampler(samplerInfo);
	}

	void Texture::copyBufferToImage(vk::Buffer buffer, uint32_t width, uint32_t height)
	{
		vk::UniqueCommandBuffer command = core::Utilities::beginSingleTimeCommands(device.getLogicalDevice(), commandPool.get());

		vk::BufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D(0, 0, 0);
		region.imageExtent = vk::Extent3D(width, height, 1);

		command.get().copyBufferToImage(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			region
		);

		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), command);
	}

	void Texture::createDescriptorPool()
	{
		vk::DescriptorPoolSize poolSize{};
		poolSize.type = vk::DescriptorType::eCombinedImageSampler;
		poolSize.descriptorCount = 1;

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;

		descriptorPool = device.getLogicalDevice().createDescriptorPoolUnique(poolInfo);
	}

	void Texture::createDescriptorSetLayout()
	{
		vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &samplerLayoutBinding;

		descriptorSetLayout = device.getLogicalDevice().createDescriptorSetLayoutUnique(layoutInfo);
	}

	void Texture::createDescriptorSet()
	{
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = descriptorPool.get();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout.get();

		descriptorSet = device.getLogicalDevice().allocateDescriptorSets(allocInfo).front();

		vk::DescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		device.getLogicalDevice().updateDescriptorSets(descriptorWrite, nullptr);
	}
}
