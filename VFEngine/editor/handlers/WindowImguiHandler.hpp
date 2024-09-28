#pragma once
#include "imguiHandler/ImguiWindowHandler.hpp"
#include "OffScreen.hpp"

namespace handlers {
	class WindowImguiHandler
	{
	private:
		interface::OffScreen& offscreen;
	public:
		explicit WindowImguiHandler(interface::OffScreen& offscreen);
		~WindowImguiHandler() = default;

		void init() const;
		void cleanUp() const;
	};
}


