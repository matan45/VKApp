#pragma once
#include "imguiHandler/ImguiWindowHandler.hpp"

namespace handlers {
	class WindowImguiHandler
	{
	public:
		explicit WindowImguiHandler() = default;
		~WindowImguiHandler() = default;

		void init() const;
		void cleanUp() const;
	};
}


