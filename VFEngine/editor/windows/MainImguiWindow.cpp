#include "MainImguiWindow.hpp"

namespace windows {
	MainImguiWindow::MainImguiWindow() :isOpen{true}
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		windowFlags = window_flags;
	}

	void MainImguiWindow::draw()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin("Vulkan Engine", &isOpen, windowFlags)) {
			ImGui::PopStyleVar(1);

			ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None);

			menuBar();
		}
		ImGui::End();
	}

	void MainImguiWindow::menuBar()
	{
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Level")) {}
				else if (ImGui::MenuItem("Open Level")) {}
				else if (ImGui::MenuItem("Save Level")) {}
				else if (ImGui::MenuItem("Exit")) {
					isOpen = false;//add close window support
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings")) {
				if (ImGui::MenuItem("Editor Camera")) {}
				else if (ImGui::MenuItem("Layout Style")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Import")) {
				if (ImGui::MenuItem("Meshes")) {
					//TODO use glfw glfwSetDropCallback(window, drop_callback);
					/* for import settings
					if (ImGui::Button("Show Modal Popup"))
			{
				ImGui::OpenPopup("Modal Popup");
			}

			if (ImGui::BeginPopupModal("Modal Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("This is a modal popup!");

				if (ImGui::Button("Close"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
					*/
					
				}
				else if (ImGui::MenuItem("Textures")) {
				/*	std::vector<std::pair<std::wstring, std::wstring>> fileTypes = {
						{ L"Text Files (*.txt)", L"*.txt" },
						{ L"Image Files (*.png;*.jpg;*.bmp)", L"*.png;*.jpg;*.bmp" },
						{ L"All Files (*.*)", L"*.*" }
					};
					std::vector<std::string> bla;
					bla.push_back(fileDialog.openFileDialog(fileTypes));
					controllers::Import::importFiles(bla);*/
				}
				else if (ImGui::MenuItem("Audio")) {
				}
				else if (ImGui::MenuItem("Animation")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}
