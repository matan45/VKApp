#pragma once
#include "OffScreen.hpp"
#include <vector>

namespace core {
	class Device;
	class SwapChain;
	class CommandPool;
}

namespace render {
	class RenderPassHandler;
}

namespace imguiPass {
	

	class OffScreenViewPort
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;
		core::CommandPool* commandPool{ nullptr };

		render::RenderPassHandler* renderPassHandler{ nullptr };

		vk::Sampler sampler;
		std::vector<OffscreenResources> offscreenResources;

	public:
		explicit OffScreenViewPort(core::Device& device, core::SwapChain& swapChain);
		~OffScreenViewPort();

		void init();
		//use std::founction for draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
		//but we cant because of the images
		vk::DescriptorSet render();
		void cleanUp() const;

	private:
		void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

		void createOffscreenResources();
		void createImage(vk::Format format, vk::ImageUsageFlags usage, vk::Image& image, vk::DeviceMemory& deviceMemory) const;
		void createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView) const;
		void updateDescriptorSets(vk::DescriptorSet& descriptorSet, const vk::ImageView& imageView) const;
		void createSampler();
	};
}

