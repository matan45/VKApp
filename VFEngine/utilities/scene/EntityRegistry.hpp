#pragma once
#include <entt/entt.hpp>

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
