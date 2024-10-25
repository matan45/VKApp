#pragma once
#include <string>
#include "config/Config.hpp"
#include "resource/Types.hpp"

namespace types {
	class Texture
	{
	public:
		void loadFromFile(const importConfig::ImportFiles& file, std::string_view fileName, std::string_view location) const;

	private:
		void saveToFile(std::string_view fileName, std::string_view location, const resource::TextureData& textureData) const;
	};
}

