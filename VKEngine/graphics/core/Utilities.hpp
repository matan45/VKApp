#pragma once
#include <optional>
#include <vulkan/vulkan.hpp>

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

namespace core {

	struct QueueFamilyIndices {
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> graphicsAndComputeFamily;

		bool isComplete() const {
			return presentFamily.has_value() && graphicsAndComputeFamily.has_value();
		}
	};

	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	class Utilities
	{
	private:
		Utilities() = delete;
		~Utilities() = delete;
	public:
		static QueueFamilyIndices findQueueFamiliesFromDevice(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
		static SwapchainSupportDetails querySwapchainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
	};
}

