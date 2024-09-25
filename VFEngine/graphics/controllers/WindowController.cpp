#include "WindowController.hpp"
#include "Graphics.hpp"
#include "../core/RenderManager.hpp"
#include "../window/Window.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"

namespace controllers {

	WindowController::WindowController() : window{ Graphics::getWindow() },
		swapChain{ Graphics::getSwapChain() }, device{ Graphics::getDevice() }
	{
		renderManager = new core::RenderManager(device, swapChain, window);
		
	}

	void WindowController::render()
	{
		renderManager->render();
	}


	void WindowController::windowPollEvents() const
	{
		window.pollEvents();
	}

	bool WindowController::windowShouldClose() const
	{
		return window.shouldClose();
	}

	bool WindowController::isWindowResized() const
	{
		return window.isWindowResized();
	}

	void WindowController::reSize() const
	{
		//handle resize window
		swapChain.recreate(window.getWidth(), window.getHeight());
		renderManager->recreate(window.getWidth(), window.getHeight());

		window.resetResizeFlag();
	}
	

	WindowController::~WindowController()
	{
		delete renderManager;
	}

	void WindowController::init()
	{
		renderManager->init();
	}

	void WindowController::cleanUp()
	{
		renderManager->cleanUp();
	}

}
