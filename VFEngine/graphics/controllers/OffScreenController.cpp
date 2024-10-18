#include "OffScreenController.hpp"
#include "../core/VulkanContext.hpp"
#include "../imguiPass/OffScreenViewPort.hpp"

namespace controllers {

	OffScreenController::OffScreenController()
		:swapChain{ *core::VulkanContext::getSwapChain() }, device{ *core::VulkanContext::getDevice() }
	{
		offScreen = new imguiPass::OffScreenViewPort(device, swapChain);
	}

	OffScreenController::~OffScreenController()
	{
		delete offScreen;
	}

	void OffScreenController::init()
	{
		offScreen->init();
	}

	void OffScreenController::cleanUp()
	{
		offScreen->cleanUp();
	}

	void* OffScreenController::render()
	{
		return offScreen->render();
	}

	

}
