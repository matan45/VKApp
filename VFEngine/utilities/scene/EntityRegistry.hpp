#pragma once
#include <entt/entt.hpp>
//TODO need to move the scene package to core
namespace scene {
	class EntityRegistry {
	private:
		inline static entt::registry registry;

		
	public:
		inline static entt::registry& getRegistry() {
			return registry;
		}
	};
}
