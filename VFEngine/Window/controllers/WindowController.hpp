#pragma once

namespace window {
	class Window;
}

namespace controllers {

	class WindowController {
	public:
		static void init();
		static window::Window* getWindow();
		static void cleanUp();
	};
}