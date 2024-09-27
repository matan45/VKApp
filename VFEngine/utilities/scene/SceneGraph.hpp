#pragma once
#include "Entity.hpp"

namespace scene {

	class SceneGraph
	{
	private:
		Entity root;
	public:
		explicit SceneGraph();
		~SceneGraph() = default;

		entt::entity addChild(const Entity& parent, const Entity& child);
		void removeEntity(const Entity& entity);

		void moveEntity(Entity& entity, Entity& newParent);

		std::vector<Entity> findAllEntitiesByName(const std::string& name);

		void updateWorldTransforms();

		const Entity& GetRoot() const {
			return root;
		}
		
	private:
		void markTransformDirty(const Entity& entity);

		void updateChildWorldTransforms(const Entity& parent, const glm::mat4& parentWorldTransform);
	};

}


