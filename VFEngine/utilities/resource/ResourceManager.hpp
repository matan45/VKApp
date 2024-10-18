#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "Types.hpp"

namespace resource {
	class ResourceManager
	{
	private:
		std::unordered_map<std::string, std::weak_ptr<TextureData>> textureCache;
		std::unordered_map<std::string, std::weak_ptr<AudioData>> audioCache;
		std::unordered_map<std::string, std::weak_ptr<MeshData>> meshCache;

	public:
		void unloadUnusedResources();
		void unloadLevelAssets();
	};
}


