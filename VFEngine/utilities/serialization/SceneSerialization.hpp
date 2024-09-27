#pragma once
#include <string_view>

namespace scene {
	class SceneGraph;
}

namespace serialization
{
	class SceneSerialization
	{
	public:
		inline static scene::SceneGraph loadScene(std::string_view filename);
		inline static void saveScene(const scene::SceneGraph& sceneGraph, std::string_view filename);
	};
}

