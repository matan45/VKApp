#pragma once
#include <string>
#include "texture/EditorTexture.hpp"

namespace controllers {
	class EditorTextureController
	{
	public:
		static dto::EditorTexture* loadTexture(std::string_view path);
		static dto::EditorTexture* loadHdrTexture(std::string_view path);
	};
}


