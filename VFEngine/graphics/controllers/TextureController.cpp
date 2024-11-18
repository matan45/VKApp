#include "TextureController.hpp"
#include "../core/VulkanContext.hpp"

namespace controllers {

	core::Texture* TextureController::createTexture()
	{
		return new core::Texture(*core::VulkanContext::getDevice());
	}
}
