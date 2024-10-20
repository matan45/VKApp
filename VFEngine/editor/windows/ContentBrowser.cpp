#include "ContentBrowser.hpp"
#include "string/StringUtil.hpp"
#include <print/EditorLogger.hpp>
#include <IconsFontAwesome6.h>
#include <algorithm>

namespace windows {
	void ContentBrowser::draw()
	{
		if (showCreateFolderModal) {
			ImGui::OpenPopup("Create New Folder");
		}
		createNewFolderModel();


		// Draw the folder panel on the left.
		if (ImGui::Begin("Folder Structure", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
			drawFolderTree(currentPath);
		}
		ImGui::End();


		if (ImGui::Begin("Content Folder")) {

			if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
				auto parentPath = currentPath.parent_path();
				navigateTo(parentPath);
			}

			ImGui::SameLine();
			// Show current path and navigation options
			ImGui::Text("Current Path: %s", StringUtil::wstringToUtf8(currentPath.wstring()).c_str());

			ImGui::Separator();

			float panelWidth = ImGui::GetContentRegionAvail().x;
			float cellSize = PADDING + THUMBNAIL_SIZE;
			int columnCount = max(1, static_cast<int>(panelWidth / cellSize));
			ImGui::Columns(columnCount, "", false);



			handleCreateFiles();

			// Draw the file-specific window if requested.
			if (showFileWindow) {
				drawFileWindow();
			}


			// also add icons
			// Display contents of the current directory
			for (const auto& asset : assets) {
				printFilesNames(asset);

				// Detect double-click on a file to open a new window.
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left)
					&& ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
					&& (asset.type != AssetType::Other || !fs::is_directory(asset.path))) {
					selectedFile = asset.path;
					showFileWindow = true;

				}
			}
		}

		ImGui::Columns(1);
		ImGui::End();
	}

	void ContentBrowser::loadDirectory(const fs::path& path)
	{
		assets.clear();
		for (auto& entry : fs::directory_iterator(path)) {
			using enum windows::AssetType;
			Asset asset;
			asset.path = StringUtil::wstringToUtf8(entry.path().wstring());
			asset.name = StringUtil::wstringToUtf8(entry.path().filename().wstring());
			if (entry.is_directory()) {
				asset.type = Other; // Indicate it's a folder
			}
			else {
				// Determine the asset type by its file extension.
				auto ext = entry.path().extension();
				if (ext == FileExtension::textrue) {
					asset.type = Texture;
				}
				else if (ext == FileExtension::mesh) {
					asset.type = Model;
				}
				else if (ext == FileExtension::shader) {
					asset.type = Shader;
				}
				else if (ext == FileExtension::audio) {
					asset.type = Audio;
				}
				else if (ext == FileExtension::animation) {
					asset.type = Animation;
				}
				else {
					asset.type = Other;
				}
			}
			assets.push_back(asset);
		}
	}
	void ContentBrowser::printFilesNames(const Asset& asset)
	{
		switch (asset.type) {
			using enum windows::AssetType;
		case Texture:
			ImGui::Text("[Texture] %s", asset.name.c_str());
			break;
		case Model:
			ImGui::Text("[Model] %s", asset.name.c_str());
			break;
		case Audio:
			ImGui::Text("[Audio] %s", asset.name.c_str());
			break;
		case Animation:
			ImGui::Text("[Animation] %s", asset.name.c_str());
			break;
		case Shader:
			ImGui::Text("[Shader] %s", asset.name.c_str());
			break;
		case Other:
			if (fs::is_directory(asset.path)) {
				if (ImGui::Selectable((ICON_FA_FOLDER " " + asset.name).c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
					navigateTo(asset.path);
				}
			}
			else {
				ImGui::Text(ICON_FA_FILE " %s", asset.name.c_str());
			}
			break;
		}
	}

	void ContentBrowser::drawFileWindow()
	{
		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);

		if (std::string windowTitle = "File: " + StringUtil::wstringToUtf8(selectedFile.filename().wstring()); ImGui::Begin(windowTitle.c_str(), &showFileWindow)) {
			ImGui::Text("File Name: %s", StringUtil::wstringToUtf8(selectedFile.filename().wstring()).c_str());
			ImGui::Text("File Path: %s", StringUtil::wstringToUtf8(selectedFile.wstring()).c_str());

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
		try {
			if (!fs::exists(newFolderPath)) {
				fs::create_directory(newFolderPath);
				loadDirectory(currentPath); // Refresh the directory to include the new folder.
			}
			else {
				vfLogWarning("Folder already exists.");
			}
		}
		catch (const fs::filesystem_error& e) {
			ImGui::Text("Failed to create folder: %s", e.what());
		}
	}
	void ContentBrowser::createNewFolderModel()
	{
		if (showCreateFolderModal &&
			ImGui::BeginPopupModal("Create New Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			// Use a buffer initialized with the current folder name.
			char buffer[256];
			std::strncpy(buffer, newFolderName.c_str(), sizeof(buffer));
			if (ImGui::InputText("Folder Name", buffer, IM_ARRAYSIZE(buffer))) {
				newFolderName = std::string(buffer);
			}

			if (ImGui::Button("Create", ImVec2(120, 0))) {
				createNewFolder(newFolderName);
				ImGui::CloseCurrentPopup();
				showCreateFolderModal = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				showCreateFolderModal = false;
			}
			ImGui::EndPopup();
		}
	}

	void ContentBrowser::handleCreateFiles()
	{
		if (ImGui::BeginPopupContextWindow()) {
			if (ImGui::MenuItem("Create New Folder")) {
				showCreateFolderModal = true;
				newFolderName.clear(); // Clear the previous input.
			}
			if (ImGui::MenuItem("Create New File")) {
				// Logic to create a new file
			}
			if (ImGui::MenuItem("Delete File")) {
				// Logic to delete file
			}
			ImGui::EndPopup();
		}
	}

	void ContentBrowser::drawFolderTree(const fs::path& path)
	{
		for (auto& entry : fs::directory_iterator(path)) {
			if (entry.is_directory()) {
				ImGui::Text(ICON_FA_FOLDER ""); // Add folder icon before the name
				ImGui::SameLine(); // Place the folder name next to the icon
				// Use a tree node for directories
				if (ImGui::TreeNode(StringUtil::wstringToUtf8(entry.path().filename().wstring()).c_str())) {
					// If the directory is selected, navigate to it in the content browser.
					if (ImGui::IsItemClicked()) {
						navigateTo(entry.path());
					}

					// Recursively draw child directories
					drawFolderTree(entry.path());

					ImGui::TreePop(); // Close the tree node.
				}
			}
		}
	}

}
