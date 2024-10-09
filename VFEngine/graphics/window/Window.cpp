#include "Window.hpp"
#include "print/Logger.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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

		setWindowIcon("../../resources/editor/VertexForge-icon.png");
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

	void Window::setWindowIcon(std::string_view iconPath) {
		// Load image using stb_image
		//TODO move it to resource menger
		int width, height, channels;
		unsigned char* imageData = stbi_load(iconPath.data(), &width, &height, &channels, 4); // 4 channels = RGBA

		if (imageData == nullptr) {
			loggerError("Failed to load icon image!");
			return;
		}

		// Create GLFWimage and assign the loaded image data
		GLFWimage icon;
		icon.width = width;
		icon.height = height;
		icon.pixels = imageData;

		// Set the icon for the GLFW window
		glfwSetWindowIcon(window, 1, &icon);

		// Free the image data after setting the icon
		stbi_image_free(imageData);
	}

}

