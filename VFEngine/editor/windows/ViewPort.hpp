#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"

namespace windows {
	class ViewPort : public interface::imguiHandler::ImguiWindow
	{
	public:
		explicit ViewPort() = default;
		~ViewPort() override = default;

		void draw() override;
	};
}

