#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

namespace components {
	struct WorldTransform {
		glm::mat4 worldMatrix;
	};

	struct Name {
		std::string name;
	};

	struct Transform {
		glm::vec3 position{ 0.0f };
		glm::vec3 rotation{ 0.0f }; // Euler angles
		glm::vec3 scale{ 1.0f };
		bool isDirty = true;

		// Mark as dirty when transform changes
		void setPosition(const glm::vec3& newPos) {
			position = newPos;
			isDirty = true;
		}

		void setRotation(const glm::vec3& newRot) {
			rotation = newRot;
			isDirty = true;
		}

		void setScale(const glm::vec3& newScale) {
			scale = newScale;
			isDirty = true;
		}

		// Compute transformation matrix without setting isDirty
		glm::mat4 GetMatrix() const {
			auto transform = glm::mat4(1.0f);
			transform = glm::translate(transform, position);
			transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
			transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
			transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
			transform = glm::scale(transform, scale);
			return transform;
		}
	};

}
