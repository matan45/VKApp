#include "Window.hpp"
#include "log/Logger.hpp"

namespace window {
	void Window::initWindow()
	{
		if (!glfwInit())
			loggerError("Unable to initialize GLFW");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "engine test", nullptr, nullptr);

		if (window == nullptr)
			loggerError("Failed to create GLFW window");

		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		WIDTH = width;
		HEIGHT = height;
		ISRESIZE = true;
	}

	Window::Window()
	{
		WIDTH = 800;
		HEIGHT = 600;
		ISRESIZE = false;
		initWindow();
	}

	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Window::createWindowSurface(const vk::UniqueInstance& instance, vk::SurfaceKHR& surface) const
	{

	}

}

