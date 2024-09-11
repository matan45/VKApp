#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

namespace core {

	class Device
	{
	private:
		vk::UniqueInstance instance{ nullptr };
		vk::PhysicalDevice physicalDevice{ nullptr };
		vk::Device logicalDevice{ nullptr };

		vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
	};
}


