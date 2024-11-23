#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>

struct TGAImage {
    int width;
    int height;
    int channels;
    std::vector<uint8_t> pixelData;
};

inline bool writeTGA(const std::string& filename, int width, int height, int channels,
                     const std::vector<uint8_t>& pixelData)
{
    if (pixelData.size() != width * height * channels)
    {
        std::cerr << "Pixel data size does not match the specified width, height, and channels.\n";
        return false;
    }

    // TGA Header (18 bytes)
    uint8_t header[18] = {};
    header[2] = 2; // Image Type: Uncompressed true-color image
    header[12] = width & 0xFF;
    header[13] = (width >> 8) & 0xFF;
    header[14] = height & 0xFF;
    header[15] = (height >> 8) & 0xFF;
    header[16] = channels * 8; // Pixel Depth: 24 bits for RGB, 32 bits for RGBA

    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    // Write header
    file.write(reinterpret_cast<char*>(header), sizeof(header));

    // Write pixel data
    if (channels == 1)
    {
        // Grayscale: Write pixel values directly
        file.write(reinterpret_cast<const char*>(pixelData.data()), pixelData.size());
    }
    else if (channels == 2)
    {
        // Grayscale + Alpha: Write GA pairs directly
        for (size_t i = 0; i < pixelData.size(); i += 2)
        {
            uint8_t ga[2] = {pixelData[i], pixelData[i + 1]}; // G (intensity), A (alpha)
            file.write(reinterpret_cast<char*>(ga), sizeof(ga));
        }
    }
    else if (channels == 3)
    {
        // RGB: Convert to BGR
        for (size_t i = 0; i < pixelData.size(); i += channels)
        {
            uint8_t bgr[3] = {pixelData[i + 2], pixelData[i + 1], pixelData[i]};
            file.write(reinterpret_cast<char*>(bgr), sizeof(bgr));
        }
    }
    else if (channels == 4)
    {
        // RGBA: Convert to BGRA
        for (size_t i = 0; i < pixelData.size(); i += channels)
        {
            uint8_t bgra[4] = {pixelData[i + 2], pixelData[i + 1], pixelData[i], pixelData[i + 3]};
            file.write(reinterpret_cast<char*>(bgra), sizeof(bgra));
        }
    }

    file.close();
    if (!file.good())
    {
        std::cerr << "Error occurred while writing to file.\n";
        return false;
    }

    return true;
}

inline bool readTGA(const std::string& filename, TGAImage& image) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    // Read the 18-byte TGA header
    uint8_t header[18];
    file.read(reinterpret_cast<char*>(header), sizeof(header));

    // Extract image properties from the header
    int width = header[12] | (header[13] << 8);
    int height = header[14] | (header[15] << 8);
    int pixelDepth = header[16]; // Pixel depth: 8, 16, 24, or 32 bits
    int channels = pixelDepth / 8;

    if (channels != 1 && channels != 2 && channels != 3 && channels != 4) {
        std::cerr << "Unsupported pixel depth: " << pixelDepth << " bits\n";
        return false;
    }

    // Allocate memory for the pixel data
    size_t pixelDataSize = width * height * channels;
    std::vector<uint8_t> pixelData(pixelDataSize);

    // Read pixel data
    file.read(reinterpret_cast<char*>(pixelData.data()), pixelDataSize);

    // Close the file
    file.close();
    if (!file.good()) {
        std::cerr << "Error occurred while reading the file.\n";
        return false;
    }

    // Handle BGR(A) to RGB(A) conversion if needed
    if (channels == 3 || channels == 4) {
        for (size_t i = 0; i < pixelDataSize; i += channels) {
            std::swap(pixelData[i], pixelData[i + 2]); // Swap B and R
        }
    }

    // Populate the TGAImage structure
    image.width = width;
    image.height = height;
    image.channels = channels;
    image.pixelData = std::move(pixelData);

    return true;
}