#include "SceneGraph.hpp"
#include <iostream>
#include "components/Components.hpp"

namespace windows {
	SceneGraph::SceneGraph()
	{
	}

	void SceneGraph::draw()
	{
		if (ImGui::Begin("SceneGraph")) {
			// Start with the root entity
			//drawEntityNode(sceneGraph.GetRoot());
		}
		ImGui::End();

		// Show the selected entity's components in the "Details" window
		if (ImGui::Begin("Details")) {
			if (selected != entt::null) {
				drawDetails(selected);
			}
		}
		ImGui::End();
	}
	void SceneGraph::drawEntityNode(entt::entity entity)
	{
		/*auto& registry = sceneGraph.GetRegistry();
		auto test = registry.create();
		registry.emplace<components::Name>(test, "test");
		auto nameComponent = registry.get<components::Name>(test);

		// Display the entity in the tree view
		ImGuiTreeNodeFlags flags = (selected == entity ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

		if (ImGui::TreeNodeEx((void*)(intptr_t)entity, flags, "%s", nameComponent.name.c_str())) {
			// If the entity is clicked, mark it as selected
			if (ImGui::IsItemClicked()) {
				selected = entity;
			}

			// Recursively display child entities
			auto const& hierarchy = registry.get<scene::Hierarchy>(entity);
			entt::entity child = hierarchy.firstChild;
			while (child != entt::null) {
				drawEntityNode(child);  // Recursive call for each child
				child = registry.get<scene::Hierarchy>(child).nextSibling;
			}

			ImGui::TreePop();
		}*/
	}
	void SceneGraph::drawDetails(entt::entity entity)
	{
		/*auto& registry = sceneGraph.GetRegistry();

		// Display the Name component if it exists
		if (registry.all_of<components::Name>(entity)) {
			auto& nameComponent = registry.get<components::Name>(entity);
			char buffer[256];
			strcpy(buffer, nameComponent.name.c_str());

			// Allow renaming the entity
			if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
				nameComponent.name = buffer;
			}
		}

		// Display and allow editing the Transform component if it exists
		if (registry.all_of<components::Transform>(entity)) {
			auto& transformComponent = registry.get<components::Transform>(entity);

			ImGui::Text("Transform");
			ImGui::DragFloat3("Position", &transformComponent.position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transformComponent.rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &transformComponent.scale.x, 0.1f);
		}*/
	}
}
