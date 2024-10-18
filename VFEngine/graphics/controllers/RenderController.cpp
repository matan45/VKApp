#include "RenderController.hpp"
#include "../core/VulkanContext.hpp"
#include "../core/RenderManager.hpp"
#include "../window/Window.hpp"

namespace controllers {

	RenderController::RenderController() :window{ core::VulkanContext::getWindow() },
		swapChain{ *core::VulkanContext::getSwapChain() }, device{ *core::VulkanContext::getDevice() }
	{
		renderManager = new core::RenderManager(device, swapChain, window);
		
	}

	void RenderController::render()
	{
		renderManager->render();
	}

	void RenderController::reSize() const
	{
		//handle resize window
		swapChain.recreate(window->getWidth(), window->getHeight());
		renderManager->recreate(window->getWidth(), window->getHeight());
	}
	

	RenderController::~RenderController()
	{
		delete renderManager;
	}

	void RenderController::init()
	{
		renderManager->init();
	}

	void RenderController::cleanUp() const
	{
		renderManager->cleanUp();
	}

}
