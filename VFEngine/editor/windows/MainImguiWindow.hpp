#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"
#include "nfd/FileDialog.hpp"

namespace windows {
	class MainImguiWindow : public controllers::imguiHandler::ImguiWindow
	{
	private:
		int windowFlags;
		bool isOpen;
		nfd::FileDialog fileDialog;
		std::vector<std::string> files;
		bool openModal = false;
	public:
		explicit MainImguiWindow();
		~MainImguiWindow() override = default;

		void draw() override;

	private:
		void menuBar();
		void importModel();
	};
}


