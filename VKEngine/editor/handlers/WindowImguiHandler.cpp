#include "WindowImguiHandler.hpp"
#include "../windows/ConsoleLog.hpp"
#include "../windows/MainImguiWindow.hpp"
#include "../windows/ContentBrowser.hpp"
#include "../windows/Details.hpp"
#include "../windows/SceneGraph.hpp"
#include "../windows/ViewPort.hpp"


namespace handlers {
	void WindowImguiHandler::init() const
	{
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::MainImguiWindow>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ConsoleLog>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ContentBrowser>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::Details>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::SceneGraph>());
		interface::imguiHandler::ImguiWindowHandler::add(std::make_shared<windows::ViewPort>());


	}

	void WindowImguiHandler::cleanUp() const
	{
		interface::imguiHandler::ImguiWindowHandler::cleanUp();
	}
}