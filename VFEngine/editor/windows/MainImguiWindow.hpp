#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "nfd/FileDialog.hpp"
#include "CoreInterface.hpp"
#include "OffScreen.hpp"
#include <filesystem>


namespace fs = std::filesystem;
namespace windows {
	class MainImguiWindow : public controllers::imguiHandler::ImguiWindow
	{
	private:
		controllers::CoreInterface& coreInterface;
		controllers::OffScreen& offscreen;
		int windowFlags;
		//import settings
		nfd::FileDialog fileDialog;
		std::vector<std::string> files;
		std::vector<bool> isFlip;
		bool openModal = false;

		//ibl window
		fs::path selectedIBLFile;
		bool showIBLWindow = false;
	public:
		explicit MainImguiWindow(controllers::CoreInterface& coreInterface,controllers::OffScreen& offscreen);
		~MainImguiWindow() override = default;

		void draw() override;

	private:
		void menuBar();
		void importModel();

		void handleFileMenu();
		void handleSettingsMenu();
		void handleAddMenu();

		void iblWindow();
	};
}


