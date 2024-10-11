#include "ImguiWindowHandler.hpp"
#include <ranges>  // C++20 ranges
#include <algorithm> // std::ranges::remove

namespace controllers::imguiHandler {
	void ImguiWindowHandler::add(const std::shared_ptr< ImguiWindow>& imguiWindow)
	{
		imguiWindows.push_back(imguiWindow);
	}

	void ImguiWindowHandler::remove(const std::shared_ptr<ImguiWindow>& imguiWindow)
	{
		auto new_end = std::ranges::remove(imguiWindows, imguiWindow);
		imguiWindows.erase(new_end.begin(), imguiWindows.end());
	}

	void ImguiWindowHandler::draw() {
		for (const auto& window : imguiWindows) {
			if (window) {
				window->draw();
			}
		}
	}

	void ImguiWindowHandler::cleanUp() {
		imguiWindows.clear();
	}
}



