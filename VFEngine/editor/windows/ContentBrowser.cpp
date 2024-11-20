#include "ContentBrowser.hpp"
#include "files/FileUtils.hpp"
#include "resource/ResourceManager.hpp"
#include "string/StringUtil.hpp"
#include "print/EditorLogger.hpp"
#include <IconsFontAwesome6.h>
#include <algorithm>

namespace windows
{
    ContentBrowser::ContentBrowser()
    {
        navigateTo(currentPath);
        
        fileIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/file.vfImage");
        folderIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/folder.vfImage");
        textureIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/texture-file.vfImage");
        audioIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/audio-file.vfImage");
        meshIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/mesh-file.vfImage");
        glslIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/glsl-file.vfImage");
        animationIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/animation-file.vfImage");
        hdrIcon = controllers::EditorTextureController::loadTexture(
            "../../resources/editor/contentBrowser/hdr-file.vfImage");
    }

    ContentBrowser::~ContentBrowser()
    {
        delete fileIcon;
        delete folderIcon;
        delete textureIcon;
        delete audioIcon;
        delete meshIcon;
        delete glslIcon;
        delete animationIcon;
        delete hdrIcon;
    }

    void ContentBrowser::draw()
    {
        if (showCreateFolderModal)
        {
            ImGui::OpenPopup("Create New Folder");
        }
        createNewFolderModel();


        // Draw the folder panel on the left.
        if (ImGui::Begin("Folder Structure", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
        {
            drawFolderTree(currentPath);
        }
        ImGui::End();


        if (ImGui::Begin("Content Folder"))
        {
            if (ImGui::Button(ICON_FA_ARROW_LEFT))
            {
                auto parentPath = currentPath.parent_path();
                navigateTo(parentPath);
            }

            ImGui::SameLine();
            // Show current path and navigation options
            ImGui::Text("Current Path: %s", StringUtil::wstringToUtf8(currentPath.wstring()).c_str());

            // Draw search bar
            ImGui::Text("Search:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            char searchBuffer[256];
            std::strncpy(searchBuffer, searchQuery.c_str(), sizeof(searchBuffer));
            if (ImGui::InputText("##Search", searchBuffer, sizeof(searchBuffer)))
            {
                searchQuery = std::string(searchBuffer);
            }

            ImGui::Separator();

            float panelWidth = ImGui::GetContentRegionAvail().x;
            float cellSize = PADDING + THUMBNAIL_SIZE;
            int columnCount = std::max(1, static_cast<int>(panelWidth / cellSize));
            ImGui::Columns(columnCount, "", false);

            handleCreateFiles();

            // Draw the file-specific window if requested.
            if (showFileWindow)
            {
                drawFileWindow();
            }
            else
            {
                if (selectedImage)
                {
                    delete selectedImage;
                    selectedImage = nullptr;
                }
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            // Display contents of the current directory
            for (const auto& asset : assets)
            {
                if (matchesSearchQuery(asset))
                {
                    printFilesNames(asset);

                    // Detect double-click on a file to open a new window.
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
                        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
                        && (asset.type != AssetType::Other || !fs::is_directory(asset.path)))
                    {
                        selectedFile = asset.path;
                        selectedType = asset.type;
                        showFileWindow = true;
                        if (selectedType == AssetType::Texture)
                        {
                            selectedImage = controllers::EditorTextureController::loadTexture(
                                StringUtil::wstringToUtf8(selectedFile.wstring()));
                        }
                        else if (selectedType == AssetType::HDR)
                        {
                            selectedImage = controllers::EditorTextureController::loadHdrTexture(
                                StringUtil::wstringToUtf8(selectedFile.wstring()));
                        }
                    }
                }
            }
            ImGui::PopStyleColor();
        }

        ImGui::Columns(1);
        ImGui::End();
    }

    void ContentBrowser::loadDirectory(const fs::path& path)
    {
        assets.clear();
        for (auto& entry : fs::directory_iterator(path))
        {
            using enum windows::AssetType;
            Asset asset;
            asset.path = StringUtil::wstringToUtf8(entry.path().wstring());
            asset.name = StringUtil::wstringToUtf8(entry.path().filename().wstring());
            if (entry.is_directory())
            {
                asset.type = Other; // Indicate it's a folder
            }
            else
            {
                // Determine the asset type by its file extension.
                resource::FileType ext = resource::ResourceManager::readHeaderFile(entry.path().string());

                if (ext == resource::FileType::TEXTURE)
                {
                    asset.type = Texture;
                }
                else if (ext == resource::FileType::HDR)
                {
                    asset.type = HDR;
                }
                else if (ext == resource::FileType::MESH)
                {
                    asset.type = Model;
                }
                else if (ext == resource::FileType::SHADER)
                {
                    asset.type = Shader;
                }
                else if (ext == resource::FileType::AUDIO)
                {
                    asset.type = Audio;
                }
                else if (ext == resource::FileType::ANIMATION)
                {
                    asset.type = Animation;
                }
                else
                {
                    asset.type = Other;
                }
            }
            assets.push_back(asset);
        }
    }

    void ContentBrowser::printFilesNames(const Asset& asset)
    {
        switch (asset.type)
        {
            using enum windows::AssetType;
        case Texture:
            ImGui::BeginGroup();
            ImGui::Image(textureIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            ImGui::TextWrapped("%s", asset.name.c_str());
            ImGui::EndGroup();
            break;
        case HDR:
            ImGui::BeginGroup();
            ImGui::Image(hdrIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            ImGui::TextWrapped("%s", asset.name.c_str());
            ImGui::EndGroup();
            break;
        case Model:
            ImGui::BeginGroup();
            ImGui::Image(meshIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            ImGui::TextWrapped("%s", asset.name.c_str());
            ImGui::EndGroup();
            break;
        case Audio:
            ImGui::BeginGroup();
            ImGui::Image(audioIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            ImGui::TextWrapped("%s", asset.name.c_str());
            ImGui::EndGroup();
            break;
        case Animation:
            ImGui::BeginGroup();
            ImGui::Image(animationIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            ImGui::TextWrapped("%s", asset.name.c_str());
            ImGui::EndGroup();
            break;
        case Shader:
            ImGui::BeginGroup();
            ImGui::Image(glslIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
            ImGui::TextWrapped("%s", asset.name.c_str());
            ImGui::EndGroup();
            break;
        case Other:
            if (fs::is_directory(asset.path))
            {
                ImGui::BeginGroup();
                std::string folderName = asset.name;
                if (ImGui::ImageButton(folderName.c_str(), folderIcon->getDescriptorSet(),
                                       ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE)))
                {
                    navigateFolder = true;
                }

                ImGui::TextWrapped("%s", folderName.c_str());
                ImGui::EndGroup();

                if (navigateFolder)
                {
                    navigateFolder = false;
                    navigateTo(asset.path);
                }
            }
            else
            {
                ImGui::BeginGroup();
                ImGui::Image(fileIcon->getDescriptorSet(), ImVec2(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
                ImGui::Text("%s", asset.name.c_str());
                ImGui::EndGroup();
            }
            break;
        }

        ImGui::NextColumn();
    }

    void ContentBrowser::drawFileWindow()
    {
        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

        if (std::string windowTitle = "File: " + StringUtil::wstringToUtf8(selectedFile.filename().wstring());
            ImGui::Begin(windowTitle.c_str(), &showFileWindow))
        {
            ImGui::Text("File Name: %s", StringUtil::wstringToUtf8(selectedFile.filename().wstring()).c_str());
            ImGui::Text("File Path: %s", StringUtil::wstringToUtf8(selectedFile.wstring()).c_str());
            ImGui::Separator();

            if(selectedType == AssetType::Texture|| selectedType == AssetType::HDR)
            {
                ImGui::Image(selectedImage->getDescriptorSet(), ImVec2(200, 200));
            }

            // Add more information or options specific to the file.
            // For example, if the file is an image, you could display it.
            // If it's a text file, you could show its contents.
            //if (selectedFile.extension() == L".txt") {}
        }
        ImGui::End();
    }

    void ContentBrowser::createNewFolder(const std::string& folderName)
    {
        fs::path newFolderPath = currentPath / folderName;
        try
        {
            if (!fs::exists(newFolderPath))
            {
                fs::create_directory(newFolderPath);
                loadDirectory(currentPath); // Refresh the directory to include the new folder.
            }
            else
            {
                vfLogWarning("Folder already exists.");
            }
        }
        catch (const fs::filesystem_error& e)
        {
            ImGui::Text("Failed to create folder: %s", e.what());
        }
    }

    void ContentBrowser::createNewFolderModel()
    {
        if (showCreateFolderModal &&
            ImGui::BeginPopupModal("Create New Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            // Use a buffer initialized with the current folder name.
            char buffer[256];
            std::strncpy(buffer, newFolderName.c_str(), sizeof(buffer));
            if (ImGui::InputText("Folder Name", buffer, IM_ARRAYSIZE(buffer)))
            {
                newFolderName = std::string(buffer);
            }

            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                createNewFolder(newFolderName);
                ImGui::CloseCurrentPopup();
                showCreateFolderModal = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                showCreateFolderModal = false;
            }
            ImGui::EndPopup();
        }
    }

    void ContentBrowser::handleCreateFiles()
    {
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Create New Folder"))
            {
                showCreateFolderModal = true;
                newFolderName.clear(); // Clear the previous input.
            }
            if (ImGui::MenuItem("Create New File"))
            {
                // Logic to create a new file
            }
            if (ImGui::MenuItem("Delete File"))
            {
                // Logic to delete file
            }
            if (ImGui::MenuItem("Rename File"))
            {
                // Logic to Rename file
            }
            ImGui::EndPopup();
        }
    }

    void ContentBrowser::drawFolderTree(const fs::path& path)
    {
        for (auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                ImGui::Text(ICON_FA_FOLDER ""); // Add folder icon before the name
                ImGui::SameLine(); // Place the folder name next to the icon
                // Use a tree node for directories
                if (ImGui::TreeNode(StringUtil::wstringToUtf8(entry.path().filename().wstring()).c_str()))
                {
                    // If the directory is selected, navigate to it in the content browser.
                    if (ImGui::IsItemClicked())
                    {
                        navigateTo(entry.path());
                    }

                    // Recursively draw child directories
                    drawFolderTree(entry.path());

                    ImGui::TreePop(); // Close the tree node.
                }
            }
        }
    }

    bool ContentBrowser::matchesSearchQuery(const Asset& asset) const
    {
        if (searchQuery.empty())
        {
            return true; // If no search term is entered, show all assets.
        }

        // Convert both the asset name and the search query to lowercase for case-insensitive comparison.
        std::string assetNameLower = StringUtil::toLower(asset.name);
        std::string searchQueryLower = StringUtil::toLower(searchQuery);

        return assetNameLower.find(searchQueryLower) != std::string::npos;
    }
}
