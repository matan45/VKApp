#include "MainLoop.hpp"
#include "Graphics.hpp"
#include "WindowController.hpp"
#include "../interface/imguiHandler/ImguiWindowHandler.hpp"
#include "time/Timer.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace core {

	MainLoop::MainLoop()
	{
		controllers::Graphics::createContext();
		windowController = new controllers::WindowController();
	}

	void MainLoop::init()
	{
		windowController->init();
		engineTime::Timer::initialize();
	}

	void MainLoop::run()
	{
		while (!windowController->windowShouldClose()) {
			windowController->windowPollEvents();

			engineTime::Timer::update();

			if (windowController->isWindowResized()) {
				windowController->reSize();
			}

			newFrame();
			editorDraw();
			endFrame();

			windowController->render();
		}
	}

	void MainLoop::cleanUp()
	{
		windowController->cleanUp();
		controllers::Graphics::destroyContext();
	}


	MainLoop::~MainLoop()
	{
		delete windowController;
	}

	void MainLoop::newFrame() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void MainLoop::endFrame() const
	{
		ImGui::EndFrame();
	}

	void MainLoop::editorDraw() const
	{
		interface::imguiHandler::ImguiWindowHandler::draw();
	}

}
