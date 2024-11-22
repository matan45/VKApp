#include "EditorTexture.hpp"
#include "TextureController.hpp"

namespace dto
{
    EditorTexture::EditorTexture(core::Texture* texture): texture{texture}
    {
        width = texture->getImageData().width;
        height = texture->getImageData().height;
        numbersOfChannels = texture->getImageData().numbersOfChannels;
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
