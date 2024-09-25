#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"

namespace windows {
	class Details : public interface::imguiHandler::ImguiWindow
	{
	public:
		explicit Details() = default;
		~Details() override = default;

		void draw() override;
	};
}

