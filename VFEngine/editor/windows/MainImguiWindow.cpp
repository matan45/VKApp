#include "MainImguiWindow.hpp"
#include "Import.hpp"
#include "print/EditorLogger.hpp"

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
		if (openModal) {
			ImGui::OpenPopup("Import Model");
		}
		
		importModel();
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
				if (ImGui::MenuItem("Import")) {
					files.clear();
					std::vector<std::pair<std::wstring, std::wstring>> fileTypes = {
							{ L"Model Files (*.obj;*.fbx;*.dae;*.gltf)", L"*.obj;*.fbx;*.dae;*.gltf" },
							{ L"Image Files (*.png;*.jpg;*.jpeg;*.hdr)", L"*.png;*.jpg;*.jpeg;*.hdr" },
							{ L"Audio Files (*.wav;*.ogg:*.mp3)", L"*.wav;*.ogg;*.mp3" }
					};
					try
					{
						files.push_back(fileDialog.openFileDialog(fileTypes));
						if (!files.empty()) {
							openModal = true;
						}
					}
					catch (...)
					{
						vfLogInfo("native file dalog close");
					}
					
				}
				else if (ImGui::MenuItem("Editor Camera")) {}
				else if (ImGui::MenuItem("Layout Style")) {}
				ImGui::EndMenu();
			}
			
			ImGui::EndMainMenuBar();
		}
	}
	void MainImguiWindow::importModel()
	{
		// Check if the popup modal is open
		if (ImGui::BeginPopupModal("Import Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Files Dropped:");

			// Display the dropped files in the modal popup
			for (const auto& file : files)
			{
				ImGui::BulletText("%s", file.c_str());
			}

			if (ImGui::Button("Continue"))
			{
				controllers::Import::importFiles(files);
				ImGui::CloseCurrentPopup(); // Close the popup
				openModal = false; // Reset the flag
			}

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Close").x - ImGui::GetStyle().FramePadding.x * 2);

			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup(); // Close the popup
				openModal = false; // Reset the flag
			}

			ImGui::EndPopup(); // End the modal
		}
	
	}
}
