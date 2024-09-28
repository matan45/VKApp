#pragma once

namespace controllers {
	class WindowController;
}


namespace core {
	class MainLoop
	{
	private:
		controllers::WindowController* windowController;
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

