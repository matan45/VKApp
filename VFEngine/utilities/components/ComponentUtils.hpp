#pragma once
#include "Components.hpp"

namespace components{
        // Update the view matrix based on the camera's position, rotation, and direction
        void updateViewMatrix(const glm::vec3& position, const glm::vec3& rotation) {
            // Create a transformation matrix based on position and rotation.
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
            transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
            transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
            transform = glm::translate(transform, -position);

            // View matrix is the inverse of the transformation matrix.
            viewMatrix = glm::inverse(transform);
        }
}