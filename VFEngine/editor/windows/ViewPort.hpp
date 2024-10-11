#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "OffScreen.hpp"

#include <memory>
#include <imgui.h>


namespace windows {
	class ViewPort : public controllers::imguiHandler::ImguiWindow
	{
	private:
		controllers::OffScreen& offscreen;
	public:
		explicit ViewPort(controllers::OffScreen& offscreen);
		~ViewPort() override = default;

		void draw() override;
	};
}

