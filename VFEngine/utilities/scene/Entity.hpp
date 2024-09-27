#pragma once
#include "EntityRegistry.hpp"
#include "../components/Components.hpp"
#include "../log/EditorLogger.hpp"

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <ranges>  // C++20 ranges
#include <algorithm> // std::ranges::remove


namespace scene {
	class Entity
	{
	private:
		entt::entity entityHandle{ entt::null };  // Entity handle
		std::string name;
		std::vector<Entity> childrens;

	public:
		explicit Entity(const std::string& name)
			: entityHandle(EntityRegistry::getRegistry().create()), name(name) {
			addComponent<components::Transform>();
		}

		// Default constructor creates an invalid entity
		Entity() = default;
		Entity(const Entity& other) = default;  // Let the compiler generate copy constructor
		Entity(Entity&&) = default;             // Let the compiler generate move constructor
		Entity& operator=(const Entity& other) = default; // Let the compiler generate copy assignment
		Entity& operator=(Entity&& other) = default;      // Let the compiler generate move assignment
		~Entity() = default;

		// Add a component to the entity
		template<typename T, typename... Args>
		T& addComponent(Args&&... args) {
			if (!isValid()) {
				vfLogError("Trying to add a component to an invalid entity.");
			}
			T& component = EntityRegistry::getRegistry().emplace<T>(entityHandle, std::forward<Args>(args)...);
			return component;
		}

		// Add or replace a component
		template<typename T, typename... Args>
		T& addOrReplaceComponent(Args&&... args) {
			if (!isValid()) {
				vfLogError("Trying to add or replace a component in an invalid entity.");
			}
			T& component = EntityRegistry::getRegistry().emplace_or_replace<T>(entityHandle, std::forward<Args>(args)...);
			return component;
		}

		// Get a component
		template<typename T>
		T& getComponent() {
			if (!hasComponent<T>()) {
				vfLogError("Entity does not have the requested component.");
			}
			return EntityRegistry::getRegistry().get<T>(entityHandle);
		}

		// Check if the entity has a specific component
		template<typename T>
		bool hasComponent() const {
			return EntityRegistry::getRegistry().all_of<T>(entityHandle);
		}

		// Remove a component
		template<typename T>
		void removeComponent() {
			if (!isValid()) {
				vfLogError("Trying to remove a component from an invalid entity.");
			}
			EntityRegistry::getRegistry().remove<T>(entityHandle);
		}

		// Check if the entity is valid (i.e., not null)
		bool isValid() const {
			return entityHandle != entt::null;
		}

		// Get the entity handle
		entt::entity getHandle() const {
			return entityHandle;
		}

		// Entity comparison operators
		bool operator==(const Entity& other) const {
			return entityHandle == other.entityHandle;
		}

		bool operator!=(const Entity& other) const {
			return !(*this == other);
		}

		// Get the entity name
		const std::string& getName() const {
			return name;
		}

		// Set the entity name
		void setName(std::string_view newName) {
			name = newName;
		}

		void addChildren(const Entity& entity) {
			childrens.push_back(entity);
		}

		void removeChildren(const Entity& entity) {
			auto new_end = std::ranges::remove(childrens, entity);
			childrens.erase(new_end.begin(), childrens.end());
		}

		// Get the list of children
		const std::vector<Entity>& getChildren() const {
			return childrens;
		}
	};
}


