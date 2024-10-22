#pragma once
#include "Device.hpp"
#include "SwapChain.hpp"
#include "print/Logger.hpp"

#include <memory>

namespace window {
	class Window;
}

namespace core {
	class VulkanContext
	{
	private:
		inline static std::unique_ptr<Device> device;
		inline static std::unique_ptr<SwapChain> swapChain;
		inline static const window::Window* window;
	public:
		static void init(const window::Window* windowGlfw);
		static void cleanup();

		static Device* getDevice() { 
			loggerAssert(device == nullptr || device.get() == nullptr, "device is not initiated");
			return device.get(); }

		static SwapChain* getSwapChain() { 
			loggerAssert(swapChain == nullptr || swapChain.get() == nullptr, "swapChain is not initiated");
			return swapChain.get(); }

		static const window::Window* getWindow() {
			loggerAssert(window == nullptr, "window is not initiated");
			return window;
		}

	};
}


