#pragma once

namespace controllers {
	class RenderController;
}

namespace window {
	class Window;
}

namespace core {
	class MainLoop
	{
	private:
		controllers::RenderController* renderController;
		window::Window* mainWindow;
	public:
		explicit MainLoop();
		~MainLoop();

		void init();
		void run();
		void cleanUp();

	private:
		void newFrame() const;
		void endFrame() const;
		void editorDraw() const;
	};

}

