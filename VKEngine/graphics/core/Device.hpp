// Define VK_NO_PROTOTYPES before including Vulkan headers

#include <vulkan/vulkan.hpp>
#include <optional>
#include <vector>
#include <array>

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

namespace window {
	class Window;
}

namespace core {

	struct QueueFamilyIndices {
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> graphicsAndComputeFamily;

		bool isComplete() const {
			return presentFamily.has_value() && graphicsAndComputeFamily.has_value();
		}
	};

	class Device
	{
	private:
		window::Window& window;

		vk::UniqueInstance instance{ nullptr };
		vk::PhysicalDevice physicalDevice{ nullptr };
		vk::Device logicalDevice{ nullptr };

		vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
		vk::DispatchLoaderDynamic dldi;

		vk::SurfaceKHR surface{ nullptr };
		vk::Queue presentQueue{ nullptr };
		vk::Queue graphicsAndComputeQueue{ nullptr };

		const std::array<const char*, 1> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::array<const char*, 1> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		// Private functions for setup and initialization
		void createInstance();
		std::vector<const char*> getRequiredExtensions() const;
		void createDebugMessenger();
		void pickPhysicalDevice();
		void createLogicalDevice();
		bool checkValidationLayerSupport() const;

		bool isDeviceSuitable(const vk::PhysicalDevice& device) const;
		bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device) const;
		QueueFamilyIndices findQueueFamiliesFromDevice(const vk::PhysicalDevice& device) const;

	public:
		explicit Device(window::Window& window);
		~Device() = default;

		void init();
		void cleanUp();
	};

}