#pragma once
#include "imguiHandler/ImguiWindowHandler.hpp"
#include "OffScreen.hpp"
#include "CoreInterface.hpp"

namespace handlers {
	class WindowImguiHandler
	{
	private:
		controllers::OffScreen& offscreen;
		controllers::CoreInterface& coreInterface;
	public:
		explicit WindowImguiHandler(controllers::OffScreen& offscreen, controllers::CoreInterface& coreInterface);
		~WindowImguiHandler() = default;

		void init() const;
		void cleanUp() const;

	};
}


