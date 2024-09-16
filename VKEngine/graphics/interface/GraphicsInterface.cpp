#include "GraphicsInterface.hpp"
#include "../window/Window.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"

namespace interface
{
	GraphicsInterface::GraphicsInterface() :window{ new window::Window() },
		device{ new core::Device(*window) }, swapChain{ new core::SwapChain(*device) }
	{

	}

	GraphicsInterface::~GraphicsInterface()
	{
		delete swapChain;
		delete device;
		delete window;
	}

	void GraphicsInterface::init() {
		window->initWindow();
		device->init();
	}

	void GraphicsInterface::cleanup()
	{
		window->cleanup();
		device->cleanUp();
	}

	void GraphicsInterface::windowPollEvents() const
	{
		window->pollEvents();
	}

	bool GraphicsInterface::windowShouldClose() const
	{
		return window->shouldClose();
	}

	bool GraphicsInterface::isWindowResized() const
	{
		return window->isWindowResized();
	}

	void GraphicsInterface::reSize() const
	{
		//handle resize window
		swapChain->recreate();


		window->resetResizeFlag();
	}
};

