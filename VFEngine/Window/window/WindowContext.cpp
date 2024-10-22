#include "WindowContext.hpp"

namespace window {
	void WindowContext::init()
	{
		mainWindow = std::make_unique<Window>();
		mainWindow->initWindow();
	}

	void WindowContext::cleanup()
	{
		mainWindow.reset();
	}

}