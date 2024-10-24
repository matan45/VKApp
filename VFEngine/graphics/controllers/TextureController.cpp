#include "TextureController.hpp"
#include "../core/VulkanContext.hpp"

namespace controllers {

	std::shared_ptr<core::Texture> TextureController::createTexture()
	{
		return textures.emplace_back(std::make_shared<core::Texture>(*core::VulkanContext::getDevice()));
	}

	void TextureController::cleanUp()
	{
		textures.clear();
	}
}
