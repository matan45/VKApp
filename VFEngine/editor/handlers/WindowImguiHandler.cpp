#include "WindowImguiHandler.hpp"
#include "../windows/ConsoleLog.hpp"
#include "../windows/MainImguiWindow.hpp"
#include "../windows/ContentBrowser.hpp"
#include "../windows/SceneGraph.hpp"
#include "../windows/ViewPort.hpp"


namespace handlers {
	WindowImguiHandler::WindowImguiHandler(controllers::OffScreen& offscreen) :offscreen{ offscreen }
	{
	}

	void WindowImguiHandler::init() const
	{
		//TODO level class
		//scene::SceneGraph mySceneGraph;  // Your SceneGraph object that manages entities

		controllers::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::MainImguiWindow>());
		controllers::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ConsoleLog>());
		controllers::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ContentBrowser>());
		controllers::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::SceneGraph>());
		controllers::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ViewPort>(offscreen));


	}

	void WindowImguiHandler::cleanUp() const
	{
		controllers::imguiHandler::ImguiWindowHandler::cleanUp();
	}
}