#include "WindowImguiHandler.hpp"
#include "../windows/ConsoleLog.hpp"
#include "../windows/MainImguiWindow.hpp"
#include "../windows/ContentBrowser.hpp"
#include "../windows/SceneGraph.hpp"
#include "../windows/ViewPort.hpp"


namespace handlers {
	WindowImguiHandler::WindowImguiHandler(interface::OffScreen& offscreen) :offscreen{ offscreen }
	{
	}

	void WindowImguiHandler::init() const
	{
		//TODO level class
		//scene::SceneGraph mySceneGraph;  // Your SceneGraph object that manages entities

		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::MainImguiWindow>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ConsoleLog>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ContentBrowser>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::SceneGraph>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ViewPort>(offscreen));


	}

	void WindowImguiHandler::cleanUp() const
	{
		interface::imguiHandler::ImguiWindowHandler::cleanUp();
	}
}