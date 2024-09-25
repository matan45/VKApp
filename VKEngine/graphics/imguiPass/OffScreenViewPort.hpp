#pragma once
#include "vulkan/vulkan.hpp"
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
	struct OffscreenResources {
		vk::Image colorImage;
		vk::DeviceMemory colorImageMemory;
		vk::ImageView colorImageView;

		vk::Image depthImage;
		vk::DeviceMemory depthImageMemory;
		vk::ImageView depthImageView;

		vk::Framebuffer framebuffer;
		vk::DescriptorSet descriptorSet;
	};

	class OffScreenViewPort
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;
		core::CommandPool* commandPool{ nullptr };

		render::RenderPassHandler* renderPassHandler{ nullptr };

		vk::RenderPass renderPass;
		vk::DescriptorPool descriptorPool;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::Sampler sampler;
		std::vector<OffscreenResources> offscreenResources;

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;
		vk::Fence renderFence;

	public:
		explicit OffScreenViewPort(core::Device& device, core::SwapChain& swapChain);
		~OffScreenViewPort();

		void init();
		vk::DescriptorSet render();
		void cleanUp() const;

	private:
		void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

		void createOffscreenResources();
		void createImage(vk::Format format, vk::ImageUsageFlags usage, vk::Image& image, vk::DeviceMemory& deviceMemory) const;
		void createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView) const;
		void createRenderPass();
		void createDescriptorSet();
		void createFramebuffer(vk::Framebuffer& framebuffer);
		void createSyncObjects();
		void createSampler();
	};
}

