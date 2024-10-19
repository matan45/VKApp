#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"
#include "nfd/FileDialog.hpp"
#include "CoreInterface.hpp"

namespace windows {
	class MainImguiWindow : public controllers::imguiHandler::ImguiWindow
	{
	private:
		controllers::CoreInterface& coreInterface;
		int windowFlags;
		nfd::FileDialog fileDialog;
		std::vector<std::string> files;
		bool openModal = false;
	public:
		explicit MainImguiWindow(controllers::CoreInterface& coreInterface);
		~MainImguiWindow() override = default;

		void draw() override;

	private:
		void menuBar();
		void importModel();

		void handleFileMenu();
		void handleSettingsMenu();
		void handleAddMenu();
	};
}


