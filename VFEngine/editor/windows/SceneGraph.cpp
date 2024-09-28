#include "SceneGraph.hpp"
#include "components/Components.hpp"

namespace windows {

	void SceneGraph::draw()
	{
		//add some styling here
		if (ImGui::Begin("SceneGraph")) {
			//TODO when right click add/remove selected entity
			// Start with the root entity (this assumes you have a SceneGraph object with a root)
			if (scene::EntityRegistry::getRegistry().valid(sceneGraphSystem->GetRoot().getHandle())) {
				drawEntityNode(sceneGraphSystem->GetRoot().getHandle());
			}
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
	
	void SceneGraph::drawEntityNode(entt::entity entity) {
		auto entityObject = scene::Entity(entity);  // Wrap entity handle in an Entity object

		std::string entityName = entityObject.getName(); // Get entity name

		ImGuiTreeNodeFlags flags = (selected == entity) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;

		bool nodeOpen = ImGui::TreeNodeEx((void*)(uint64_t)entity, flags, "%s", entityName.c_str());

		// Select the entity when clicked
		if (ImGui::IsItemClicked()) {
			selected = entity;
		}

		// Drag source: Start dragging the entity
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("DND_ENTITY", &entity, sizeof(entt::entity));  // Tag it with "DND_ENTITY"
			ImGui::Text("Move %s", entityName.c_str());  // Show name of entity being dragged
			ImGui::EndDragDropSource();
		}

		// Drag target: Drop onto this entity (to make it a parent)
		drawDragDropTarget(entity);

		// If the entity has children, recursively draw them
		if (nodeOpen) {
			if (entityObject.hasComponent<components::ChildrenComponent>()) {
				for (auto child : entityObject.getChildren()) {
					drawEntityNode(child.getHandle());  // Recursively draw child nodes
				}
			}
			ImGui::TreePop();
		}
	}

	void SceneGraph::drawDragDropTarget(entt::entity entity) const {
		auto entityObject = scene::Entity(entity);

		//root node should not move and the root name need to be quince
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				// Retrieve the dropped entity handle
				entt::entity droppedEntity = *(entt::entity*)payload->Data;

				// Set the parent-child relationship
				if (droppedEntity != entity) {
					auto droppedEntityObj = scene::Entity(droppedEntity);
					droppedEntityObj.addOrReplaceComponent<components::ParentComponent>().parent = entity;
					entityObject.addChildren(droppedEntityObj);  // Make this entity the new parent
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	void SceneGraph::drawDetails(entt::entity entity) const {
		auto entityObject = scene::Entity(entity);
		//maybe need pushID here
		// Display the name component first if it exists
		if (entityObject.hasComponent<components::Name>()) {
			auto const& nameComponent = entityObject.getComponent<components::Name>();
			char buffer[256];
			strcpy(buffer, nameComponent.name.c_str());
			if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
				entityObject.setName(buffer);  // Update the name
			}
		}

		ImGui::Separator(); // Separate components

		// Dynamically iterate over all attached components
		drawDynamicComponent(entity);
	}

	void SceneGraph::drawDynamicComponent(entt::entity entity) const {
		auto& registry = scene::EntityRegistry::getRegistry();
		//TODO when right click add/remove components also maybe need pushID here
		// Handle known component types
		if (registry.all_of<components::Transform>(entity)) {
			//TODO add collapsingHeader
			auto& transform = registry.get<components::Transform>(entity);
			ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
			transform.isDirty = true;
		}
	}

}
