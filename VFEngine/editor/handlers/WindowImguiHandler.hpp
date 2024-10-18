#pragma once
#include "imguiHandler/ImguiWindowHandler.hpp"
#include "OffScreen.hpp"

namespace handlers {
	class WindowImguiHandler
	{
	private:
		controllers::OffScreen& offscreen;
	public:
		explicit WindowImguiHandler(controllers::OffScreen& offscreen);
		~WindowImguiHandler() = default;

		void init() const;
		void cleanUp() const;
	};
}


