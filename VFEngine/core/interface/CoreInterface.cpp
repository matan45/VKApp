#include "CoreInterface.hpp"
#include "../core/MainLoop.hpp"

namespace interface {

	CoreInterface::CoreInterface() : mainLoop{ new core::MainLoop() }
	{

	}

	void CoreInterface::init()
	{
		mainLoop->init();
	}

	void CoreInterface::run() const
	{
		mainLoop->run();
	}

	void CoreInterface::cleanUp()
	{
		mainLoop->cleanUp();
	}

	CoreInterface::~CoreInterface()
	{
		delete mainLoop;
	}



};