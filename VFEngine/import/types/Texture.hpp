#pragma once
#include <string>
#include "config/Config.hpp"
#include "resource/Types.hpp"

namespace types
{
    class Texture
    {
    public:
        void loadTextureFile(const importConfig::ImportFiles& file, std::string_view fileName,
                             std::string_view location) const;
        void loadHDRFile(const importConfig::ImportFiles& file, std::string_view fileName,
                         std::string_view location) const;

    private:
        void saveToFileTexture(std::string_view fileName, std::string_view location,
                               const resource::TextureData& textureData) const;
        void saveToFileHDR(std::string_view fileName, std::string_view location,
                           const resource::HDRData& hdrData) const;
    };
}
