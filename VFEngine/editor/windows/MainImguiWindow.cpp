#include "MainImguiWindow.hpp"
#include "Import.hpp"
#include "print/EditorLogger.hpp"
#include "config/Config.hpp"
#include "files/FileUtils.hpp"
#include <imgui.h>


namespace windows
{
    MainImguiWindow::MainImguiWindow(controllers::CoreInterface& coreInterface) : coreInterface{coreInterface}
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoBackground;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        windowFlags = window_flags;
    }

    void MainImguiWindow::draw()
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Vulkan Engine", nullptr, windowFlags))
        {
            ImGui::PopStyleVar(1);

            ImGui::DockSpace(ImGui::GetID("MyDockSpace"), ImVec2(0.0f, 0.0f),
                             ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_None);

            menuBar();
        }
        ImGui::End();
    }

    void MainImguiWindow::menuBar()
    {
        if (openModal)
        {
            ImGui::OpenPopup("Import Model");
        }

        importModel();
        if (ImGui::BeginMainMenuBar())
        {
            handleFileMenu();
            handleSettingsMenu();
            handleAddMenu();
            ImGui::EndMainMenuBar();
        }
    }

    void MainImguiWindow::importModel()
    {
        // Check if the popup modal is open
        if (ImGui::BeginPopupModal("Import Model", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            std::vector<importConfig::ImportFiles> paths;
            isFlip.resize(files.size(), false);
            paths.reserve(files.size());

            ImGui::Text("Files Dropped:");
            // Display the dropped files in the modal popup
            for (int i = 0; i < files.size(); i++)
            {
                ImGui::PushID(files[i].c_str());
                importConfig::ImportConfig config;
                ImGui::BulletText("%s", files[i].c_str());
                if (files::FileUtils::isImageFile(files[i]))
                {
                    bool flip = isFlip[i];
                    ImGui::Checkbox("Flip Vertically", &flip);
                    isFlip[i] = flip;
                    config.isImageFlipVertically = isFlip[i];
                }
                importConfig::ImportFiles importFile(files[i], config);
                paths.emplace_back(importFile);
                ImGui::PopID();
            }

            if (ImGui::Button("Continue"))
            {
                controllers::Import::importFiles(paths);
                ImGui::CloseCurrentPopup(); // Close the popup
                openModal = false; // Reset the flag
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(
                ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Close").x - ImGui::GetStyle().FramePadding.x *
                2);

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup(); // Close the popup
                openModal = false; // Reset the flag
            }

            ImGui::EndPopup(); // End the modal
        }
    }

    void MainImguiWindow::handleFileMenu()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Level"))
            {
            }
            else if (ImGui::MenuItem("Open Level"))
            {
            }
            else if (ImGui::MenuItem("Save Level"))
            {
            }
            else if (ImGui::MenuItem("Exit"))
            {
                coreInterface.closeWindow();
            }
            ImGui::EndMenu();
        }
    }

    void MainImguiWindow::handleSettingsMenu()
    {
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Import"))
            {
                std::vector<std::pair<std::wstring, std::wstring>> fileTypes = {
                    {L"Model Files (*.obj;*.fbx;*.dae;*.gltf)", L"*.obj;*.fbx;*.dae;*.gltf"},
                    {L"Image Files (*.png;*.jpg;*.jpeg;*.hdr)", L"*.png;*.jpg;*.jpeg;*.hdr"},
                    {L"Audio Files (*.wav;*.ogg:*.mp3)", L"*.wav;*.ogg;*.mp3"}
                };

                files = fileDialog.multiSelectFileDialog(fileTypes);
                if (!files.empty())
                {
                    openModal = true;
                }
            }
            else if (ImGui::MenuItem("Editor Camera"))
            {
            }
            else if (ImGui::MenuItem("Layout Style"))
            {
            }
            ImGui::EndMenu();
        }
    }

    void MainImguiWindow::handleAddMenu()
    {
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::MenuItem("IBL"))
            {
            }
            else if (ImGui::MenuItem("Terrain"))
            {
            }
            ImGui::EndMenu();
        }
    }
}
