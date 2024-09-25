#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"

namespace windows {
	class ContentBrowser : public interface::imguiHandler::ImguiWindow
	{
	public:
		explicit ContentBrowser() = default;
		~ContentBrowser() override = default;

		void draw() override;
	};
}

