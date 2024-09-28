#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "OffScreen.hpp"

#include <memory>
#include <imgui.h>


namespace windows {
	class ViewPort : public interface::imguiHandler::ImguiWindow
	{
	private:
		interface::OffScreen& offscreen;
	public:
		explicit ViewPort(interface::OffScreen& offscreen);
		~ViewPort() override = default;

		void draw() override;
	};
}

