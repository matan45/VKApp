#pragma once
#include "Entity.hpp"

namespace scene {

	class SceneGraphSystem
	{
	private:
		Entity root;
	public:
		explicit SceneGraphSystem();
		~SceneGraphSystem() = default;

		entt::entity addChild(Entity& parent,Entity& child) const;
		void removeEntity(Entity& entity);

		void moveEntity(Entity& entity, Entity& newParent) const;

		std::vector<Entity> findAllEntitiesByName(std::string_view name) const;

		void updateWorldTransforms();
		void updateCamera();

		const Entity& GetRoot() const {
			return root;
		}
		
	private:
		void markTransformDirty(Entity& entity) const;

		void updateChildWorldTransforms(Entity& parent, const glm::mat4& parentWorldTransform);

		bool isDescendant(scene::Entity& parent,scene::Entity& child) const;
	};

}


