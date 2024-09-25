#include "MainLoop.hpp"
#include "Graphics.hpp"
#include "WindowController.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace core {

	void MainLoop::init()
	{
		controllers::Graphics::createContext();
		windowController = new controllers::WindowController();
		windowController->init();
	}

	void MainLoop::run()
	{
		while (!windowController->windowShouldClose()) {
			windowController->windowPollEvents();

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
		ImGui::ShowDemoWindow();
	}

}
