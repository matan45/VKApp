#include "CoreInterface.hpp"


namespace interface {
	

	void CoreInterface::init()
	{
		graphicsInterface->init();
	}

	void CoreInterface::run() const
	{
		while (!graphicsInterface->windowShouldClose()) {
			graphicsInterface->windowPollEvents();
		}
	}

	void CoreInterface::cleanUp()
	{
		graphicsInterface->cleanup();
	}
};
