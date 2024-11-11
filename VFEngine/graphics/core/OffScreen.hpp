#pragma once
#include <vulkan/vulkan.hpp>

namespace core {
	struct ColorImage {
		vk::Image colorImage;
		vk::DeviceMemory colorImageMemory;
		vk::ImageView colorImageView;
		vk::DescriptorSet descriptorSet;
	};

	struct DepthImage {
		vk::Image depthImage;
		vk::DeviceMemory depthImageMemory;
		vk::ImageView depthImageView;
	};

	struct OffscreenResources {
		std::vector<core::ColorImage> colorImages;
		core::DepthImage depthImage;
	};
}
