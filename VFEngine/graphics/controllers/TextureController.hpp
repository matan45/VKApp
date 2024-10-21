#pragma once
#include <memory>
#include <string>
#include "../core/Texture.hpp"

namespace controllers {
	class TextureController
	{
		static inline std::vector< std::shared_ptr<core::Texture>> textures;
	public:
		static std::shared_ptr<core::Texture> createTexture();
		static void cleanUp();
	};
}


