#pragma once
#include "imguiHandler/ImguiWindowHandler.hpp"
#include "OffScreen.hpp"
#include "CoreInterface.hpp"
#include "../windows/SceneGraph.hpp"

namespace handlers {
	class WindowImguiHandler
	{
	private:
		controllers::OffScreen& offscreen;
		controllers::CoreInterface& coreInterface;
		//TODO level class
		std::shared_ptr<scene::SceneGraphSystem> sceneGraphSystem = std::make_shared<scene::SceneGraphSystem>();
	public:
		explicit WindowImguiHandler(controllers::OffScreen& offscreen, controllers::CoreInterface& coreInterface);
		~WindowImguiHandler() = default;

		void init() const;
		void cleanUp() const;

	};
}


