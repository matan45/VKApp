#include "OffScreen.hpp"
#include "OffScreenController.hpp"

namespace interface {
	OffScreen::OffScreen()
	{
		offScreenController = new controllers::OffScreenController();
	}

	OffScreen::~OffScreen()
	{
		delete offScreenController;
	}

	void OffScreen::init()
	{
		offScreenController->init();
	}

	void OffScreen::cleanUp()
	{
		offScreenController->cleanUp();
	}

	void* OffScreen::render()
	{
		return offScreenController->render();
	}
}
