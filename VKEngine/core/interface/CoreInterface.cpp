#include "CoreInterface.hpp"
#include "GraphicsInterface.hpp"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>


namespace interface {

	CoreInterface::CoreInterface() : graphicsInterface{ new GraphicsInterface() }
	{

	}

	void CoreInterface::init()
	{
		graphicsInterface->init();
	}

	void CoreInterface::run() const
	{
		while (!graphicsInterface->windowShouldClose()) {
			graphicsInterface->windowPollEvents();

			if (graphicsInterface->isWindowResized()) {
				graphicsInterface->reSize();
			}

			newFrame();
			editorDraw();
			endFrame();

			graphicsInterface->render();
		}
	}

	void CoreInterface::cleanUp()
	{
		graphicsInterface->cleanup();
	}

	CoreInterface::~CoreInterface()
	{
		delete graphicsInterface;
	}

	void CoreInterface::newFrame() const
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void CoreInterface::endFrame() const
	{
		ImGui::EndFrame();
	}

	void CoreInterface::editorDraw() const
	{

		ImGui::ShowDemoWindow();
	}

};