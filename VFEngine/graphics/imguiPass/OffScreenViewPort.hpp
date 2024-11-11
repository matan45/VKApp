#pragma once
#include "../core/OffScreen.hpp"
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
		core::OffscreenResources offscreenResources;

	public:
		explicit OffScreenViewPort(core::Device& device, core::SwapChain& swapChain);
		~OffScreenViewPort();

		void init();
		vk::DescriptorSet render();
		void cleanUp() const;

	private:
		void draw(const vk::CommandBuffer& commandBuffer) const;

		void createOffscreenResources();
		void updateDescriptorSets(vk::DescriptorSet& descriptorSet, const vk::ImageView& imageView) const;
		void createSampler();
	};
}

