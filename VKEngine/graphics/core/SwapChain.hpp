#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {
	class Device;

	struct SwapChainDepthStencil {
		vk::UniqueImage depthStencilImage;
		vk::UniqueDeviceMemory depthStencilMemory;
		vk::UniqueImageView depthStencilView;
	};

	class SwapChain
	{
	private:
		Device& device;

		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapchainImageViews;

		SwapChainDepthStencil swapchainDepthStencil{};

		vk::Format swapchainImageFormat{ vk::Format::eUndefined };
		vk::Format swapchainDepthStencilFormat{ vk::Format::eUndefined };
		vk::Extent2D swapchainExtent;

	public:
		explicit SwapChain(Device& device);
		~SwapChain() = default;

		void init(uint32_t width, uint32_t height);
		void cleanUp();

		uint32_t getImageCount() const { return static_cast<uint32_t>(swapchainImages.size()); }
		vk::SwapchainKHR getSwapchain() const { return swapchain.get(); }
		vk::Image getSwapchainImage(uint32_t imageIndex) const { return swapchainImages[imageIndex]; }
		vk::Image getDepthStencilImage() const { return swapchainDepthStencil.depthStencilImage.get(); }

		vk::Format getSwapchainDepthStencilFormat() const { return swapchainDepthStencilFormat; }
		vk::Format getSwapchainImageFormat() const { return swapchainImageFormat; }


		void recreate(uint32_t width, uint32_t height);

	private:
		void createSwapchain(uint32_t width, uint32_t height);
		void createImageViews();
		void createDepthStencil();
		vk::Format findDepthStencilFormat() const;

		vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
		vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;
		vk::Extent2D chooseSwapExtent(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& capabilities) const;
	};
}

