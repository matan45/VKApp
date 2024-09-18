#pragma once


namespace window {
	class Window;
}

namespace core {
	class Device;
	class SwapChain;
	class RenderManager;
}

namespace interface {
	class GraphicsInterface
	{
	private:
		window::Window* window{ nullptr };
		core::Device* device{ nullptr };
		core::SwapChain* swapChain{ nullptr };
		core::RenderManager* renderManager{ nullptr };
	public:
		explicit GraphicsInterface();
		~GraphicsInterface();

		void init();
		void cleanup();

		void windowPollEvents() const;
		bool windowShouldClose() const;
		bool isWindowResized() const;
		void reSize() const;
	};
};


