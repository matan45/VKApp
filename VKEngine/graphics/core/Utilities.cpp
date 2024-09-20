#include "Utilities.hpp"
#include "log/Logger.hpp"

namespace core {

	QueueFamilyIndices Utilities::findQueueFamiliesFromDevice(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
	{
		QueueFamilyIndices indices;
		const std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		if (debug) {
			loggerInfo("Found {} queue families.", queueFamilies.size());
		}

		int i = 0;
		for (const vk::QueueFamilyProperties& queueFamily : queueFamilies) {
			if (debug) {
				loggerInfo("Queue Family {}: Graphics: {}, Compute: {}, Transfer: {}",
					i,
					(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) ? "Yes" : "No",
					(queueFamily.queueFlags & vk::QueueFlagBits::eCompute) ? "Yes" : "No",
					(queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) ? "Yes" : "No"
				);
			}

			if (device.getSurfaceSupportKHR(i, surface)) {
				indices.presentFamily = i;
			}

			if ((queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) && (queueFamily.queueFlags & vk::QueueFlagBits::eCompute)) {
				indices.graphicsAndComputeFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		if (!indices.isComplete()) {
			if (debug) {
				loggerWarning("Could not find complete queue family support.");
			}
		}

		return indices;
	}

	core::SwapchainSupportDetails Utilities::querySwapchainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
	{
		SwapchainSupportDetails details;
		details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
		details.formats = device.getSurfaceFormatsKHR(surface);
		details.presentModes = device.getSurfacePresentModesKHR(surface);
		return details;
	}

	uint32_t Utilities::findMemoryType(const vk::PhysicalDevice& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		loggerError("Failed to find suitable memory type.");
	}

}

