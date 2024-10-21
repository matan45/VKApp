#include "TextureLoderController.hpp"
#include "TextureController.hpp"
#include <vector>

namespace controllers {

	void* TextureLoderController::loadTexture(std::string_view path)
	{
		auto texture = TextureController::createTexture();
		texture->loadFromFile(path);
		return texture->getDescriptorSet();
	}

	void TextureLoderController::cleanUp()
	{
		TextureController::cleanUp();
	}
}
