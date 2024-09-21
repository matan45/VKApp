#include "GraphicsInterface.hpp"
#include "../window/Window.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/RenderManager.hpp"

namespace interface
{
	GraphicsInterface::GraphicsInterface() :window{ new window::Window() },
		device{ new core::Device(*window) },
		swapChain{ new core::SwapChain(*device) }
		, renderManager{ new core::RenderManager(*device,*swapChain,*window) }
	{

	}

	void GraphicsInterface::init() {
		window->initWindow();
		device->init();
		swapChain->init(window->getWidth(), window->getHeight());
		renderManager->init();
	}

	void GraphicsInterface::cleanup()
	{
		renderManager->cleanUp();
		swapChain->cleanUp();
		device->cleanUp();
		window->cleanup();
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
		swapChain->recreate(window->getWidth(), window->getHeight());
		renderManager->recreate(window->getWidth(), window->getHeight());

		window->resetResizeFlag();
	}

	void GraphicsInterface::render()
	{
		renderManager->render();
	}

	GraphicsInterface::~GraphicsInterface()
	{
		delete renderManager;
		delete swapChain;
		delete device;
		delete window;
	}

};

