#include "SceneSerialization.hpp"
#include "../scene/SceneGraph.hpp"

namespace serialization {

	scene::SceneGraph SceneSerialization::loadScene(std::string_view filename)
	{
		return scene::SceneGraph();
	}

	void SceneSerialization::saveScene(const scene::SceneGraph& sceneGraph, std::string_view filename)
	{

	}

}