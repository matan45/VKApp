#include "Window.hpp"
#include "print/Logger.hpp"
#include "resource/TextureResource.hpp"


namespace window {
	void Window::initWindow()
	{
		if (!glfwInit()) {
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

		setWindowIcon("../../resources/editor/VertexForge-icon.vfImage");
	}

	void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		auto userWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		userWindow->width = width;
		userWindow->height = height;
		userWindow->isResized = true;
	}

	vk::SurfaceKHR Window::createWindowSurface(const vk::UniqueInstance& instance) const
	{
		VkSurfaceKHR rawSurface;
		VkResult result = glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface);
		loggerAssert(result != VK_SUCCESS,
			"failed to create window surface!");

		return vk::SurfaceKHR(rawSurface);
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

	void Window::setWindowIcon(std::string_view iconPath) {
		resource::TextureData iconData = resource::TextureResource::loadTexture(iconPath);

		// Create GLFWimage and assign the loaded image data
		GLFWimage icon;
		icon.width = iconData.width;
		icon.height = iconData.height;
		icon.pixels = iconData.textureData.data();

		// Set the icon for the GLFW window
		glfwSetWindowIcon(window, 1, &icon);
	}

}

