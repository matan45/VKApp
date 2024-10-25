#include "EditorTexture.hpp"
#include "TextureController.hpp"

namespace dto
{
    EditorTexture::EditorTexture(core::Texture* texture): texture{texture}
    {
    }

    EditorTexture::~EditorTexture()
    {
        delete texture;
    }

    void* dto::EditorTexture::getDescriptorSet() const
    {
        return texture->getDescriptorSet();
    }
}
