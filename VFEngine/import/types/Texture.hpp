#pragma once
#include <string>
#include "resource/Types.hpp"

namespace types {
	class Texture
	{
	public:
		void loadFromFile(std::string_view path, std::string_view fileName, std::string_view location) const;

	private:
		void saveToFile(std::string_view fileName, std::string_view location, const resource::TextureData& textureData) const;
	};
}

