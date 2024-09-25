#include "Window.hpp"
#include "log/Logger.hpp"


namespace window {
	void Window::initWindow()
	{
		if(!glfwInit()) {
			loggerError("Unable to initialize GLFW");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		window = glfwCreateWindow(width, height, "VertexForge", nullptr, nullptr);

		if (window == nullptr) {
			loggerError("Failed to create GLFW window");
		}

		glfwSetWindowUserPointer(window, this); // Set the user pointer to access the class
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto userWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		userWindow->width = width;
		userWindow->height = height;
		userWindow->isResized = true;
	}

	void Window::createWindowSurface(const vk::UniqueInstance& instance, vk::SurfaceKHR& surface)
	{
		VkSurfaceKHR rawSurface;
		VkResult result = glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface);
		loggerAssert(result != VK_SUCCESS,
			"failed to create window surface!");

		surface = vk::SurfaceKHR(rawSurface);
	}

	void Window::cleanup()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::pollEvents() const
	{
		glfwPollEvents();
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

}

