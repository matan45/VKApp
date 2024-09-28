#pragma once
#include <memory>
#include "CoreInterface.hpp"
#include "WindowImguiHandler.hpp"
#include "OffScreen.hpp"

namespace handlers {
	class EditorHandler
	{
	private:
		std::unique_ptr<interface::CoreInterface> coreInterface;
		interface::OffScreen* offScreenInterface;

		std::unique_ptr<WindowImguiHandler> windowImguiHandler;

	public:
		explicit EditorHandler();
		~EditorHandler();

		void init() const;
		void run() const;
		void cleanUp();
	};
}


