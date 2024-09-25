#include "Graphics.hpp"
#include "../window/Window.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"


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
		return *window;
	}

	core::Device& Graphics::getDevice()
	{
		return *device;
	}

	core::SwapChain& Graphics::getSwapChain()
	{
		return *swapChain;
	}

};

