#pragma once

namespace core {
	class MainLoop;
}

namespace interface {

	class CoreInterface
	{
	private:
		core::MainLoop* mainLoop;
	public:
		explicit CoreInterface();
		~CoreInterface();

		void init();
		void run() const;
		void cleanUp();

	};
}

