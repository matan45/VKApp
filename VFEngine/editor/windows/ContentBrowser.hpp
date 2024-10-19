#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"
#include "Import.hpp"


#include <string>
#include <filesystem>
namespace fs = std::filesystem;

namespace windows {

	enum class AssetType { Texture, Model, Audio, Animation, Shader, Other };

	struct Asset {
		std::string name;
		std::string path;
		AssetType type;
	};


	class ContentBrowser : public controllers::imguiHandler::ImguiWindow
	{
	private:
		std::vector<Asset> assets;
		fs::path currentPath = "c:\\matan";

		std::string newFolderName;
		bool showCreateFolderModal = false;

		fs::path selectedFile;
		bool showFileWindow = false;
	public:
		explicit ContentBrowser() {
			navigateTo(currentPath);
		};
		~ContentBrowser() override = default;

		void draw() override;

	private:
		void navigateTo(const fs::path& path) {
			if (fs::exists(path) && fs::is_directory(path)) {
				currentPath = path;
				controllers::Import::setLocation(currentPath.string());
				loadDirectory(currentPath);
			}
		}

		void loadDirectory(const fs::path& path);

		void printFilesNames(const Asset& asset);

		void drawFileWindow();

		void createNewFolder(const std::string& folderName);
		void createNewFolderModel();
		void handleCreateFiles();

		void drawFolderTree(const fs::path& path);
	};
}

