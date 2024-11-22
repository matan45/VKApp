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

        std::vector<float> convertFromEXRToHDR(const float* data, int width, int height) const;
        void flipImageVertically(float* imageData, int width, int height) const;
    };

    struct RGBE
    {
        uint8_t r, g, b, e; // Red, Green, Blue, Exponent
    };

    class HDRWriter
    {
    public:
        static void writeHDR(std::ofstream& file, int width, int height, int numbersOfChannels,
                             const std::vector<float>& pixels);

    private:
        static uint8_t getChannel(const std::vector<float>& pixels, int width, int y, int x, int channel);
        static RGBE encodeRGBE(float r, float g, float b);
    };
}
