#include "VulkanContext.hpp"
#include "../window/Window.hpp"

namespace core {
	void VulkanContext::init(const window::Window* windowGlfw)
	{
		device = std::make_unique<core::Device>(windowGlfw);
		swapChain = std::make_unique<core::SwapChain>(*device.get());
		device->init();
		swapChain->init(windowGlfw->getWidth(), windowGlfw->getHeight());
		window = windowGlfw;
	}

	void VulkanContext::cleanup()
	{
		swapChain->cleanUp();
		device->cleanUp();

		swapChain.reset();
		device.reset();
	}

}
