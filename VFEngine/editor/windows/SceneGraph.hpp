#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "imgui.h"

namespace windows {
	class SceneGraph : public interface::imguiHandler::ImguiWindow
	{
	public:
		explicit SceneGraph() = default;
		~SceneGraph() override = default;

		void draw() override;
	};
}

