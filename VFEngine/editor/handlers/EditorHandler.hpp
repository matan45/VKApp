#pragma once
#include <memory>
#include "CoreInterface.hpp"
#include "WindowImguiHandler.hpp"

namespace handlers {
	class EditorHandler
	{
	private:
		std::unique_ptr<interface::CoreInterface> coreInterface;
		std::unique_ptr<WindowImguiHandler> windowImguiHandler;

	public:
		explicit EditorHandler();
		~EditorHandler() = default;

		void init() const;
		void run() const;
		void cleanUp();
	};
}


