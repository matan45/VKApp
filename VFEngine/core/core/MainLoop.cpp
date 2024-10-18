#include "MainLoop.hpp"
#include "Graphics.hpp"
#include "RenderController.hpp"
#include "../controllers/imguiHandler/ImguiWindowHandler.hpp"
#include "../window/Window.hpp"
#include "time/Timer.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace core {

	MainLoop::MainLoop()
	{
		//todo use controller also in graphics
		mainWindow = new window::Window();
		mainWindow->initWindow();
		controllers::Graphics::createContext(mainWindow);
		renderController = new controllers::RenderController();
	}

	void MainLoop::init()
	{
		renderController->init();
		engineTime::Timer::initialize();
	}

	void MainLoop::run()
	{
		while (!mainWindow->shouldClose()) {
			mainWindow->pollEvents();

			engineTime::Timer::update();

			if (mainWindow->isWindowResized()) {
				renderController->reSize();
				mainWindow->resetResizeFlag();
			}

			newFrame();
			editorDraw();
			endFrame();

			renderController->render();
		}
	}

	void MainLoop::cleanUp()
	{
		renderController->cleanUp();
		controllers::Graphics::destroyContext();
	}


	MainLoop::~MainLoop()
	{
		delete renderController;
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
		controllers::imguiHandler::ImguiWindowHandler::draw();
	}

}
