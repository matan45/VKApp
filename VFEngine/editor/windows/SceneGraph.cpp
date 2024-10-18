#include "SceneGraph.hpp"
#include "components/Components.hpp"

namespace windows {

	void SceneGraph::draw()
	{
		//add some styling here
		if (ImGui::Begin("SceneGraph")) {

			// Start with the root entity (this assumes you have a SceneGraph object with a root)
			if (scene::EntityRegistry::getRegistry().valid(sceneGraphSystem->GetRoot().getHandle())) {
				drawEntityNode(sceneGraphSystem->GetRoot());
			}

			// Right-click context menu for adding/removing entities
			if (ImGui::BeginPopupContextWindow()) {
				// Menu item to add a new entity
				if (ImGui::MenuItem("Add New Entity")) {
					scene::Entity newEntity("New Entity");

					// Add as a child to the selected entity if there is one, else add to root
					if (selected != entt::null) {
						scene::Entity parent(selected);
						sceneGraphSystem->addChild(parent, newEntity);
					}
					else {
						scene::Entity root = sceneGraphSystem->GetRoot();
						sceneGraphSystem->addChild(root, newEntity);
					}
				}

				// Prevent deleting the root entity
				if (selected != entt::null && selected != sceneGraphSystem->GetRoot().getHandle()) {
					if (ImGui::MenuItem("Remove Selected Entity")) {
						// Remove the selected entity from the scene graph
						scene::Entity selectedToRemove(selected);
						sceneGraphSystem->removeEntity(selectedToRemove);
						selected = entt::null;  // Clear the selection after removal
					}
				}

				ImGui::EndPopup();
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

	void SceneGraph::drawEntityNode(scene::Entity entity) {
		ImGui::PushID(static_cast<int>(entity.getHandle()));

		std::string entityName = entity.getName(); // Get entity name

		ImGuiTreeNodeFlags flags = (selected == entity.getHandle()) ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= ImGuiTreeNodeFlags_OpenOnArrow;
		

		bool nodeOpen = ImGui::TreeNodeEx((void*)(uint64_t)entity.getHandle(), flags, "%s", entityName.c_str());

		// Select the entity when clicked
		if (ImGui::IsItemClicked()) {
			selected = entity.getHandle();
		}

		// Drag source: Start dragging the entity
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("DND_ENTITY", &entity, sizeof(scene::Entity));  // Tag it with "DND_ENTITY"
			ImGui::Text("Move %s", entityName.c_str());  // Show name of entity being dragged
			ImGui::EndDragDropSource();
		}

		// Drag target: Drop onto this entity (to make it a parent)
		drawDragDropTarget(entity);


		// If the entity has children, recursively draw them
		if (nodeOpen) {
			if (entity.hasComponent<components::ChildrenComponent>()) {
				for (auto child : entity.getChildren()) {
					drawEntityNode(child);  // Recursively draw child nodes
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	void SceneGraph::drawDragDropTarget(scene::Entity entity) const {

		//root node should not move and the root name need to be quince
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY")) {
				// Retrieve the dropped entity handle
				scene::Entity droppedEntity = *(scene::Entity*)payload->Data;

				// Set the parent-child relationship
				if (droppedEntity.getHandle() != entity.getHandle()) {
					sceneGraphSystem->moveEntity(droppedEntity, entity);
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
		drawDynamicComponent(entityObject);
	}

	void SceneGraph::drawDynamicComponent(scene::Entity entity) const {
		auto& registry = scene::EntityRegistry::getRegistry();
		// Handle known component types
		if (registry.all_of<components::Transform>(entity.getHandle())) {
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
				auto& transform = registry.get<components::Transform>(entity.getHandle());
				ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
				ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
				ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
				transform.isDirty = true;
			}
		}
	}

}
