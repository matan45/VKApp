#include "Graphics.hpp"
#include "../window/Window.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "print/Logger.hpp"


namespace controllers
{

	void Graphics::init() {
		window->initWindow();
		device->init();
		swapChain->init(window->getWidth(), window->getHeight());

	}

	void Graphics::cleanup()
	{
		swapChain->cleanUp();
		device->cleanUp();
		window->cleanup();
	}

	void Graphics::createContext()
	{
		window = new window::Window();
		device = new core::Device(*window);
		swapChain = new core::SwapChain(*device);

		init();
	}

	void Graphics::destroyContext()
	{
		cleanup();

		delete swapChain;
		delete device;
		delete window;
	}

	window::Window& Graphics::getWindow()
	{
		loggerAssert(window == nullptr, "window is not initiated");
		return *window;
	}

	core::Device& Graphics::getDevice()
	{
		loggerAssert(device == nullptr, "device is not initiated");
		return *device;
	}

	core::SwapChain& Graphics::getSwapChain()
	{
		loggerAssert(swapChain == nullptr, "swapChain is not initiated");
		return *swapChain;
	}

};

