#include "Texture.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <bit>  // For std::bit_cast
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace types {
	void Textures::loadFromFile(std::string_view path) const
	{
		resource::TextureData textureData;

		// Load image using stb_image
		int width;
		int height;
		int channels;
		unsigned char* imageData = stbi_load(path.data(), &width, &height, &channels, 0);

		if (!imageData) {
			std::cerr << "Failed to load texture: " << path << std::endl;
			return ; // Return
		}

		// Store texture information
		textureData.width = static_cast<uint32_t>(width);
		textureData.height = static_cast<uint32_t>(height);
		textureData.version = { 1, 0, 0 }; // Set version (modify as needed)

		// Determine texture format based on channels
		switch (channels) {
			case 1: textureData.textureFormat = "R"; break; // Red channel only
			case 3: textureData.textureFormat = "RGB"; break;
			case 4: textureData.textureFormat = "RGBA"; break;
			default: textureData.textureFormat = "Unknown"; break;
		}

		// Store raw texture data into the vector
		textureData.textureData = std::vector<unsigned char>(imageData, imageData + (width * height * channels));

		// Free the image data once copied to the structure
		stbi_image_free(imageData);

		saveToFile(path, textureData);
	}

	void Textures::saveToFile(std::string_view path, const resource::TextureData& textureData) const
	{
		// Open the file in binary mode
		std::ofstream outFile(path.data(), std::ios::binary);

		if (!outFile) {
			std::cerr << "Failed to open file for writing: " << path << std::endl;
			return;
		}

		// Write version
		outFile.write(std::bit_cast<const char*>(&textureData.version.major), sizeof(textureData.version.major));
		outFile.write(std::bit_cast<const char*>(&textureData.version.minor), sizeof(textureData.version.minor));
		outFile.write(std::bit_cast<const char*>(&textureData.version.patch), sizeof(textureData.version.patch));

		// Write width and height
		outFile.write(std::bit_cast<const char*>(&textureData.width), sizeof(textureData.width));
		outFile.write(std::bit_cast<const char*>(&textureData.height), sizeof(textureData.height));

		// Write texture format size and then the format string itself
		auto formatLength = static_cast<uint32_t>(textureData.textureFormat.size());
		outFile.write(std::bit_cast<const char*>(&formatLength), sizeof(formatLength)); // Write the length of the format string
		outFile.write(textureData.textureFormat.c_str(), formatLength);                    // Write the format string itself

		// Write the size of the texture data and then the raw texture data
		auto textureDataSize = static_cast<uint32_t>(textureData.textureData.size());
		outFile.write(std::bit_cast<const char*>(&textureDataSize), sizeof(textureDataSize)); // Write the size of the texture data
		outFile.write(std::bit_cast<const char*>(textureData.textureData.data()), textureDataSize); // Write the raw texture data

		// Close the file
		outFile.close();

		std::cout << "Texture data successfully saved to " << path << std::endl;
	}
}
