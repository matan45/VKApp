#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {
	class Device;

	struct SwapChainDepthStencil {
		vk::Image depthStencilImage;
		vk::DeviceMemory depthStencilMemory;
		vk::ImageView depthStencilView;
	};

	class SwapChain
	{
	private:
		core::Device& device;

		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapchainImageViews;

		SwapChainDepthStencil swapchainDepthStencil{};

		vk::Format swapchainImageFormat{ vk::Format::eUndefined };
		vk::Format swapchainDepthStencilFormat{ vk::Format::eUndefined };
		vk::Extent2D swapchainExtent;

	public:
		explicit SwapChain(core::Device& device);
		~SwapChain() = default;

		void init(uint32_t width, uint32_t height);
		void cleanUp();

		void recreate(uint32_t width, uint32_t height);

	private:
		void createSwapchain(uint32_t width, uint32_t height);
		void createImageViews();
		void createDepthStencil();
		vk::Format findDepthStencilFormat() const;

		vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
		vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;
		vk::Extent2D chooseSwapExtent(uint32_t width, uint32_t height,const vk::SurfaceCapabilitiesKHR& capabilities) const;
	};
}

