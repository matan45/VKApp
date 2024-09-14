#include "GraphicsInterface.hpp"
#include "../window/Window.hpp"
#include "../core/Device.hpp"

namespace interface
{
	GraphicsInterface::GraphicsInterface() :window{ new window::Window() }, device{ new core::Device(*window) }
	{

	}

	GraphicsInterface::~GraphicsInterface()
	{
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
};


