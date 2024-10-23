#include "EditorTextureController.hpp"
#include "TextureController.hpp"
#include <vector>

namespace controllers {

	void* EditorTextureController::loadTexture(std::string_view path)
	{
		auto texture = TextureController::createTexture();
		texture->loadFromFile(path);
		return texture->getDescriptorSet();
	}

	void EditorTextureController::cleanUp()
	{
		TextureController::cleanUp();
	}
}
