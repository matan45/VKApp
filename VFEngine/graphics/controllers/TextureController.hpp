#pragma once
#include <memory>
#include <string>
#include "../core/Texture.hpp"

namespace controllers {
	class TextureController
	{
	public:
		static core::Texture* createTexture();
	};
}


