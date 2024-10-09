#pragma once
#include <vulkan/vulkan.hpp>

namespace imguiPass {
	struct OffscreenResources {
		vk::Image colorImage;
		vk::DeviceMemory colorImageMemory;
		vk::ImageView colorImageView;

		vk::Image depthImage;
		vk::DeviceMemory depthImageMemory;
		vk::ImageView depthImageView;

		vk::DescriptorSet descriptorSet;
	};
}
