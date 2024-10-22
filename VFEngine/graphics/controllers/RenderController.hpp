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
	class RenderController
	{
	private:
		const window::Window* window;
		core::SwapChain& swapChain;
		core::Device& device;
		core::RenderManager* renderManager;

	public:
		explicit RenderController();
		~RenderController();

		void init();
		void cleanUp() const;

		void reSize() const;

		void render();
	};
}



