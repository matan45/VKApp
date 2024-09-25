#pragma once
#include <vector>
#include <memory>
#include "ImguiWindow.hpp"

namespace interface::imguiHandler {

	class ImguiWindowHandler
	{
	private:
		inline static std::vector<std::shared_ptr<ImguiWindow>> imguiWindows;
	public:
		explicit ImguiWindowHandler() = default;
		~ImguiWindowHandler() = default;

		static void add(const std::shared_ptr< ImguiWindow>& imguiWindow);
		static void remove(const std::shared_ptr< ImguiWindow>& imguiWindow);
		static void draw();
		static void cleanUp();
	};

};


