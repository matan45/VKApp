#pragma once
#include <string_view>

namespace scene {
	class SceneGraphSystem;
}

namespace serialization
{
	class SceneSerialization
	{
	public:
		inline static scene::SceneGraphSystem loadScene(std::string_view filename);
		inline static void saveScene(const scene::SceneGraphSystem& sceneGraph, std::string_view filename);
	};
}

