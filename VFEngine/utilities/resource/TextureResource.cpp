#include "TextureResource.hpp"
#include <fstream>
#include <iostream>
#include <bit>  // For std::bit_cast

namespace resource {

	TextureData TextureResource::loadTexture(std::string_view path)
	{
		resource::TextureData textureData;

		// Open the file in binary mode
		std::ifstream inFile(path.data(), std::ios::binary);
		if (!inFile) {
			std::cerr << "Failed to open file for reading: " << path << std::endl;
			return textureData; // Return an empty TextureData on failure
		}

		// Read version
		inFile.read(std::bit_cast<char*>(&textureData.version.major), sizeof(textureData.version.major));
		inFile.read(std::bit_cast<char*>(&textureData.version.minor), sizeof(textureData.version.minor));
		inFile.read(std::bit_cast<char*>(&textureData.version.patch), sizeof(textureData.version.patch));

		// Read width and height
		inFile.read(std::bit_cast<char*>(&textureData.width), sizeof(textureData.width));
		inFile.read(std::bit_cast<char*>(&textureData.height), sizeof(textureData.height));

		// Read the texture format (length and string)
		uint32_t formatLength;
		inFile.read(std::bit_cast<char*>(&formatLength), sizeof(formatLength)); // Read the length of the format string
		textureData.textureFormat.resize(formatLength);                            // Resize the string buffer
		inFile.read(&textureData.textureFormat[0], formatLength);                  // Read the format string itself

		// Read the texture data size and then the texture data itself
		uint32_t textureDataSize;
		inFile.read(std::bit_cast<char*>(&textureDataSize), sizeof(textureDataSize)); // Read the size of the texture data
		textureData.textureData.resize(textureDataSize);                                  // Resize the textureData vector
		inFile.read(std::bit_cast<char*>(textureData.textureData.data()), textureDataSize); // Read the raw texture data

		// Close the file
		inFile.close();

		std::cout << "Texture data successfully loaded from " << path << std::endl;
		return textureData;
	}
}

