#pragma once

namespace window {
	class Window;
}

namespace core {
	class SwapChain;
	class Device;
	class RenderManager;
}

namespace controllers {
	class WindowController
	{
	private:
		window::Window& window;
		core::SwapChain& swapChain;
		core::Device& device;
		core::RenderManager* renderManager;

	public:
		explicit WindowController();
		~WindowController();

		void init();
		void cleanUp();

		void windowPollEvents() const;
		bool windowShouldClose() const;
		bool isWindowResized() const;
		void reSize() const;

		void render();
	};
}



