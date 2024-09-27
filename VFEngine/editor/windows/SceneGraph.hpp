#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "components/Components.hpp"
#include "scene/Entity.hpp"

#include <imgui.h>
#include <entt/entt.hpp>

namespace windows {
	class SceneGraph : public interface::imguiHandler::ImguiWindow
	{
	private:
		entt::entity selected = entt::null;  // Currently selected entity
	
	public:
		explicit SceneGraph();
		~SceneGraph() override = default;

		void draw() override;

	private:
		void drawEntityNode(entt::entity entity);  // Helper function to display entity hierarchy
		void drawDetails(entt::entity entity);     // Helper function to display selected entity's details
	};
}

