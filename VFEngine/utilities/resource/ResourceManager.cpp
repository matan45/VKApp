#include "ResourceManager.hpp"

namespace resource {
	void ResourceManager::unloadUnusedResources()
	{
		std::erase_if(textureCache, [](const auto& pair) {
			return pair.second.expired();  // Remove the entry if the resource is no longer referenced
			});
		std::erase_if(audioCache, [](const auto& pair) {
			return pair.second.expired();  // Remove the entry if the resource is no longer referenced
			});
		std::erase_if(meshCache, [](const auto& pair) {
			return pair.second.expired();  // Remove the entry if the resource is no longer referenced
			});
	}

	void ResourceManager::unloadLevelAssets()
	{
		textureCache.clear();  
		audioCache.clear();
		meshCache.clear();
	}

}
