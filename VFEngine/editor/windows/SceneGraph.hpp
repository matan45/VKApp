#pragma once
#include "imguiHandler/ImguiWindow.hpp"
#include "scene/SceneGraphSystem.hpp"
#include <memory>

namespace windows {
	class SceneGraph : public controllers::imguiHandler::ImguiWindow
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
		void drawEntityNode(scene::Entity entity); 
		void drawDetails(entt::entity entity) const;
		void drawDynamicComponent(scene::Entity entity) const;

		void drawDragDropTarget(scene::Entity entity) const;
	};
}

