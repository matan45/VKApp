#include "EditorHandler.hpp"

namespace handlers {
	EditorHandler::EditorHandler() :coreInterface{ std::make_unique<interface::CoreInterface>() },
		offScreenInterface{ new interface::OffScreen() },
		windowImguiHandler{ std::make_unique<WindowImguiHandler>(*offScreenInterface) }
	{

	}

	EditorHandler::~EditorHandler()
	{
		delete offScreenInterface;
	}

	void EditorHandler::init() const
	{
		//need also to load the level here
		coreInterface->init();
		windowImguiHandler->init();
		offScreenInterface->init();
	}

	void EditorHandler::run() const
	{
		coreInterface->run();
	}

	void EditorHandler::cleanUp()
	{
		windowImguiHandler->cleanUp();
		offScreenInterface->cleanUp();
		coreInterface->cleanUp();
	}
}

