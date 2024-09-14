
#include "Device.hpp"
#include "log/Logger.hpp"
#include "../window/Window.hpp"

#include <unordered_set>


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		loggerWarning("Validation layer: {}", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		loggerError("Validation layer ERROR: {}", pCallbackData->pMessage);
	}
	else {
		loggerInfo("Validation layer: {}", pCallbackData->pMessage);
	}
	return VK_FALSE;
}


namespace core {

	Device::Device(window::Window& window) : window{ window } {}

	void Device::init()
	{
		createInstance();
		createDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void Device::cleanUp()
	{
		if (logicalDevice) {
			logicalDevice.waitIdle();
			logicalDevice.destroy();
		}

		if (debug && debugMessenger) {
			instance->destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
		}

		if (surface) {
			instance->destroySurfaceKHR(surface);
		}
	}

	void Device::createInstance()
	{

		vk::ApplicationInfo appInfo{
		   "Vulkan App",
		   VK_MAKE_VERSION(1, 0, 0),
		   "Engine",
		   VK_MAKE_VERSION(1, 0, 0),
		   VK_API_VERSION_1_3
		};

		std::vector<const char*> extensions = getRequiredExtensions();

		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (debug) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		try {
			instance = vk::createInstanceUnique(createInfo);
		}
		catch (const vk::SystemError& err) {
			loggerError("Failed to create Vulkan instance: {}", err.what());
			throw std::runtime_error("Vulkan instance creation failed.");
		}

		if (debug) {
			for (const auto& extension : vk::enumerateInstanceExtensionProperties()) {
				loggerInfo("Available extension: {}", extension.extensionName);
			}
		}

		if (debug) {
			uint32_t version{ 0 };
			if (vk::Result result = vk::enumerateInstanceVersion(&version); result == vk::Result::eSuccess) {
				loggerInfo("Vulkan API version: {}.{}.{}",
					VK_API_VERSION_MAJOR(version),
					VK_API_VERSION_MINOR(version),
					VK_API_VERSION_PATCH(version));
			}
			else {
				loggerError("Failed to enumerate Vulkan instance version. Error code: {}", vk::to_string(result));
			}


			loggerInfo("Vulkan API version: {}.{}.{}",
				VK_API_VERSION_MAJOR(version), VK_API_VERSION_MINOR(version), VK_API_VERSION_PATCH(version));

			if (!checkValidationLayerSupport()) {
				loggerError("Validation layers requested, but not available!");
				throw std::runtime_error("Validation layers requested, but not available!");
			}
		}
	}

	std::vector<const char*> Device::getRequiredExtensions() const
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (debug) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Device::createDebugMessenger()
	{

		using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
		if (!debug) return;

		dldi = vk::DispatchLoaderDynamic(*instance, vkGetInstanceProcAddr);

		vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
		createInfo.messageType = eGeneral | eValidation | ePerformance;
		createInfo.pfnUserCallback = debugCallback;

		try {
			debugMessenger = instance->createDebugUtilsMessengerEXT(createInfo, nullptr, dldi);
		}
		catch (const vk::SystemError& err) {
			loggerError("Failed to set up debug messenger: {}", err.what());
			throw;
		}
	}

	void Device::pickPhysicalDevice()
	{
		const std::vector<vk::PhysicalDevice> devices = instance->enumeratePhysicalDevices();

		if (debug) {
			loggerInfo("Found {} devices with Vulkan support.", devices.size());
		}

		for (const vk::PhysicalDevice& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				if (debug) {
					loggerInfo("Selected physical device: {}", physicalDevice.getProperties().deviceName);
				}
				break;
			}
		}

		if (!physicalDevice) {
			loggerError("Failed to find a suitable GPU!");
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	void Device::createLogicalDevice()
	{
		surface = window.createWindowSurface(instance);
		const QueueFamilyIndices indices = findQueueFamiliesFromDevice();
		const float queuePriority = 1.0f;

		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = indices.graphicsAndComputeFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = { queueCreateInfo };

		vk::PhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		vk::DeviceCreateInfo createInfo{};
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (debug) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		try {
			logicalDevice = physicalDevice.createDevice(createInfo);
			graphicsAndComputeQueue = logicalDevice.getQueue(indices.graphicsAndComputeFamily.value(), 0);
			presentQueue = logicalDevice.getQueue(indices.presentFamily.value(), 0);
		}
		catch (const vk::SystemError& err) {
			loggerError("Failed to create logical device: {}", err.what());
			throw;
		}
	}

	bool Device::checkValidationLayerSupport() const
	{
		if (uint32_t layerCount = 0; vk::enumerateInstanceLayerProperties(&layerCount, nullptr) == vk::Result::eSuccess) {
			std::vector<vk::LayerProperties> availableLayers(layerCount);
			if (vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data()) == vk::Result::eSuccess) {
				for (const auto& layer : availableLayers) {
					loggerInfo("Available validation layer: {}", layer.layerName);
				}
			}
			else {
				loggerError("Failed to retrieve Vulkan instance layers.");
			}
		}
		else {
			loggerError("Failed to count Vulkan instance layers.");
		}

		return true;
	}

	bool Device::isDeviceSuitable(const vk::PhysicalDevice& device) const
	{
		const QueueFamilyIndices indices = findQueueFamiliesFromDevice();
		const bool extensionsSupported = checkDeviceExtensionSupport(device);
		const vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();

		return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
	}

	bool Device::checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const
	{
		std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {
			requiredExtensions.erase(extension.extensionName);
		}

		if (!requiredExtensions.empty()) {
			if (debug) {
				for (const auto& ext : requiredExtensions) {
					loggerError("Required device extension not found: {}", ext);
				}
			}
			return false;
		}

		return true;
	}

	QueueFamilyIndices Device::findQueueFamiliesFromDevice() const {
		QueueFamilyIndices indices;
		const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

		if (debug) {
			loggerInfo("Found {} queue families.", queueFamilies.size());
		}

		int i = 0;
		for (const vk::QueueFamilyProperties& queueFamily : queueFamilies) {
			if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
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

		if (!indices.isComplete() && debug) {
			loggerWarning("Could not find complete queue family support.");
		}

		return indices;
	}

}
