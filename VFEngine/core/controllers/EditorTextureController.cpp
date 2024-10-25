#include "EditorTextureController.hpp"
#include "TextureController.hpp"

namespace controllers {

	dto::EditorTexture* EditorTextureController::loadTexture(std::string_view path)
	{
		auto texture = TextureController::createTexture();
		texture->loadFromFile(path);
		return new dto::EditorTexture(texture);
	}
}
