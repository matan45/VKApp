#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"

namespace window {
	class Window
	{
	private:
		GLFWwindow* window{ nullptr };
		inline static bool ISRESIZE;
		inline static int WIDTH;
		inline static int HEIGHT;

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	public:
		explicit Window();
		~Window() = default;

		vk::SurfaceKHR createWindowSurface(const vk::UniqueInstance& instance) const;

		void initWindow();
		void cleanup();
		void pollEvents() const;
		bool shouldClose() const;
	};
}


