#pragma once

namespace core {
    class Texture;
}

namespace dto
{
    class EditorTexture
    {
    private:
        core::Texture* texture;

    public:
        explicit EditorTexture(core::Texture* texture);
        ~EditorTexture();
        void* getDescriptorSet() const;
    };
}
