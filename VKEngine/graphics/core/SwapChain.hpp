#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

namespace core {
	class Device;

	class SwapChain
	{
	private:
		core::Device& device;

		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchainImages;
		std::vector<vk::ImageView> swapchainImageViews;

		vk::Format swapchainImageFormat{ vk::Format::eUndefined };
		vk::Extent2D swapchainExtent;

	public:
		explicit SwapChain(core::Device& device);
		~SwapChain() = default;

		void init();
		void cleanUp();

		void recreate();

	private:
		void createSwapchain();
		void createImageViews();

		vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
		vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) const;
		vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;
	};
}

