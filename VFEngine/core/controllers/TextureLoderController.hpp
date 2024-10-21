#pragma once
#include <string>

namespace controllers {
	class TextureLoderController
	{
	public:
		static void* loadTexture(std::string_view path);
		static void cleanUp();
	};
}


