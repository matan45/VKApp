#include "WindowImguiHandler.hpp"
#include "../windows/ConsoleLog.hpp"
#include "../windows/MainImguiWindow.hpp"
#include "../windows/ContentBrowser.hpp"
#include "../windows/SceneGraph.hpp"
#include "../windows/ViewPort.hpp"


namespace handlers
{
    WindowImguiHandler::WindowImguiHandler(controllers::OffScreen& offscreen,
                                           controllers::CoreInterface& coreInterface) : offscreen{offscreen},
        coreInterface{coreInterface}
    {
    }

    void WindowImguiHandler::init() const
    {
        //TODO level class
        //scene::SceneGraph mySceneGraph;  // Your SceneGraph object that manages entities

        controllers::imguiHandler::ImguiWindowHandler::add(
            std::make_shared<windows::MainImguiWindow>(coreInterface, offscreen));
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
