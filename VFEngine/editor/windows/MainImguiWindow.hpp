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
	public:
		explicit MainImguiWindow();
		~MainImguiWindow() override = default;

		void draw() override;

	private:
		void menuBar();
	};
}


