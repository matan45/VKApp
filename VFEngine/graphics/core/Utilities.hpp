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

	struct ImageInfoRequest
	{
		const vk::Device& logicalDevice;
		const vk::PhysicalDevice& physicalDevice;
		uint32_t width;
		uint32_t height;
		vk::Format format;
		vk::ImageTiling tiling;
		vk::ImageUsageFlags usage;
		vk::MemoryPropertyFlags properties;

		explicit ImageInfoRequest(const vk::Device& logicalDevice, const vk::PhysicalDevice& physicalDevice)
			: logicalDevice{ logicalDevice }, physicalDevice{ physicalDevice }
		{}
	};

	struct BufferInfoRequest
	{
		const vk::Device& logicalDevice;
		const vk::PhysicalDevice& physicalDevice;
		vk::DeviceSize size;
		vk::BufferUsageFlags usage;
		vk::MemoryPropertyFlags properties;

		explicit BufferInfoRequest(const vk::Device& logicalDevice, const vk::PhysicalDevice& physicalDevice)
			: logicalDevice{ logicalDevice }, physicalDevice{ physicalDevice }
		{}
	};

	class Utilities
	{
	private:
		Utilities() = delete;
		~Utilities() = delete;
	public:
		static QueueFamilyIndices findQueueFamiliesFromDevice(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
		static SwapchainSupportDetails querySwapchainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
		static uint32_t findMemoryType(const vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		static vk::UniqueCommandBuffer beginSingleTimeCommands(const vk::Device& device, const vk::CommandPool& commandPool);
		static void endSingleTimeCommands(const vk::Queue& queue, const vk::UniqueCommandBuffer& commandBuffer);

		static void transitionImageLayout(const vk::CommandBuffer& commandBuffer, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask);

		static void createBuffer(const BufferInfoRequest& bufferInfo, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
		static void createImage(const ImageInfoRequest& imageInfo, vk::Image& image, vk::DeviceMemory& imageMemory);
		static void createImageView(const vk::Device& device, const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView);

	};
}

