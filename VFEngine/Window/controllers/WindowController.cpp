#include "WindowController.hpp"
#include "../window/WindowContext.hpp"

namespace controllers {
	void WindowController::init() {
		window::WindowContext::init();
	}

	void WindowController::cleanUp() {
		window::WindowContext::cleanup();
	}

	window::Window* WindowController::getWindow() {
		return window::WindowContext::getWindow();
	}
}