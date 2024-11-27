#pragma once
#include <string>
#include "Types.hpp"

namespace resource
{
    class TextureResource
    {
    private:
        inline static const size_t chunkSize = 1024 * 1024;

    public:
        static TextureData loadTexture(std::string_view path);
        static HDRData loadHDR(std::string_view path);
    };

    struct RGBE
    {
        uint8_t r, g, b, e; // Red, Green, Blue, Exponent
    };

    class HDRReader
    {
    public:
        static void readHDR(std::ifstream& file, int width, int height, std::vector<float>& pixels);

    private:
        static void decodeRGBE(const RGBE& rgbe, float& r, float& g, float& b);
    };

    struct TGAImage {
        int width;
        int height;
        int channels;
        std::vector<uint8_t> pixelData;
    };
    
    class TGAReader
    {
    public:
        static void readTGA(std::ifstream& file, int width, int height, int numbersOfChannels,
                            std::vector<unsigned char>& pixelData);
    };
}
