#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"

namespace windows {
	class MainImguiWindow : public controllers::imguiHandler::ImguiWindow
	{
	private:
		int windowFlags;
		bool isOpen;
	public:
		explicit MainImguiWindow();
		~MainImguiWindow() override = default;

		void draw() override;

	private:
		void menuBar();
	};
}


