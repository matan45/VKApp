#pragma once

namespace controllers {
	class WindowController;
}

namespace handlers {
	class WindowHandler
	{
	private:
		controllers::WindowController* windowController;

	public:
		explicit WindowHandler();
		~WindowHandler();


	};
}


