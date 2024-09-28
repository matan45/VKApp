#include "OffScreenController.hpp"
#include "Graphics.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../imguiPass/OffScreenViewPort.hpp"

namespace controllers {

	OffScreenController::OffScreenController()
		:swapChain{ Graphics::getSwapChain() }, device{ Graphics::getDevice() }
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
