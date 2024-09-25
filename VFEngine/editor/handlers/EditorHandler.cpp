#include "EditorHandler.hpp"

namespace handlers {
	EditorHandler::EditorHandler() :coreInterface{ std::make_unique<interface::CoreInterface>() }
		,windowImguiHandler{ std::make_unique<WindowImguiHandler>() }
	{

	}

	void EditorHandler::init() const
	{
		coreInterface->init();
		windowImguiHandler->init();
	}

	void EditorHandler::run() const
	{
		coreInterface->run();
	}

	void EditorHandler::cleanUp()
	{
		windowImguiHandler->cleanUp();
		coreInterface->cleanUp();
	}
}

