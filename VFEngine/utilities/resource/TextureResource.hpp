#pragma once
#include <string>
#include "Types.hpp"

namespace resource {
	class TextureResource
	{
	public:
		static TextureData loadTexture(std::string_view path);
	};

}

