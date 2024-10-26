#pragma once
#include <string>
#include "config/Config.hpp"
#include "resource/Types.hpp"
struct aiScene;

namespace types {
	class Mesh
	{
	public:
		void loadFromFile(const importConfig::ImportFiles& file, std::string_view fileName, std::string_view location) const;

	private:
		
		void saveToFile(std::string_view location, std::string_view fileName, const resource::MeshesData&  meshesData) const;
		void processAssimpScene(const aiScene* scene, std::string_view fileName, std::string_view location) const;
	};

}


