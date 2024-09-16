#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"

namespace window {
	class Window
	{
	private:
		GLFWwindow* window{ nullptr };
		bool isResized{ false }; // Non-static resize flag
		int width{ 800 };
		int height{ 600 };


		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	public:
		explicit Window() = default;
		~Window() = default;

		void createWindowSurface(const vk::UniqueInstance& instance, vk::SurfaceKHR& surface);

		void initWindow();
		void cleanup();
		void pollEvents() const;
		bool shouldClose() const;
		bool isWindowResized() const { return isResized; }
		void resetResizeFlag() { isResized = false; }
	};
}


