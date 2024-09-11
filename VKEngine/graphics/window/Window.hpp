#pragma once

#include <vulkan/vulkan.hpp>
#include "GLFW/glfw3.h"

namespace window {
	class Window
	{
	private:
		GLFWwindow* window{};
		inline static bool ISRESIZE;
		inline static int WIDTH;
		inline static int HEIGHT;

		void initWindow();

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	public:
		explicit Window();
		~Window();

		void createWindowSurface(const vk::UniqueInstance& instance, vk::SurfaceKHR& surface) const;
	};
}


