#include "CoreInterface.hpp"
#include "GraphicsInterface.hpp"


namespace interface {

	CoreInterface::CoreInterface() : graphicsInterface{ new GraphicsInterface() }
	{

	}

	void CoreInterface::init()
	{
		graphicsInterface->init();
	}

	void CoreInterface::run() const
	{
		while (!graphicsInterface->windowShouldClose()) {
			graphicsInterface->windowPollEvents();

			if (graphicsInterface->isWindowResized()) {
				graphicsInterface->reSize();
			}

			graphicsInterface->render();
		}
	}

	void CoreInterface::cleanUp()
	{
		graphicsInterface->cleanup();
	}

	CoreInterface::~CoreInterface()
	{
		delete graphicsInterface;
	}
};
