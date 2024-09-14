#pragma once
#include <memory>


namespace window {
	class Window;
}

namespace core {
	class Device;
}

namespace interface {
	class GraphicsInterface
	{
	private:
		window::Window* window{ nullptr };
		core::Device* device{ nullptr };
	public:
		explicit GraphicsInterface();
		~GraphicsInterface();

		void init();
		void cleanup();

		void windowPollEvents() const;
		bool windowShouldClose() const;
	};
};


