#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "scene/SceneGraphSystem.hpp"

#include <imgui.h>
#include <memory>

namespace windows {
	class SceneGraph : public interface::imguiHandler::ImguiWindow
	{
	private:
		entt::entity selected = entt::null;  // Currently selected entity
		//TODO be part of the level class maybe need to be shared when swap levels?
		std::unique_ptr<scene::SceneGraphSystem> sceneGraphSystem = std::make_unique<scene::SceneGraphSystem>();
	public:
		explicit SceneGraph() = default;
		~SceneGraph() override = default;

		void draw() override;

	private:
		void drawEntityNode(entt::entity entity);  // Helper function to display entity hierarchy
		void drawDetails(entt::entity entity) const;     // Helper function to display selected entity's details
		void drawDynamicComponent(entt::entity entity) const; // Helper for dynamically rendering components

		void drawDragDropTarget(entt::entity entity) const; // Helper for drag-and-drop logic
	};
}

