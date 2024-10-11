#pragma once
#include <string>
#include "resource/Types.hpp"

namespace types {
	class Textures
	{
	public:
		void loadFromFile(std::string_view path) const;

	private:
		void saveToFile(std::string_view path, const resource::TextureData& textureData) const;
	};
}

