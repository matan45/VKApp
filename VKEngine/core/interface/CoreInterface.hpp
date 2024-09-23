#pragma once

namespace interface {
	class GraphicsInterface;

	class CoreInterface
	{
	private:
		GraphicsInterface* graphicsInterface;
	public:
		explicit CoreInterface();
		~CoreInterface();

		void init();
		void run() const;
		void cleanUp();

	private:
		void newFrame() const;
		void endFrame() const;
		void editorDraw() const;
	};
}

