#include "TextureResource.hpp"
#include "../print/EditorLogger.hpp"

#include <fstream>
#include <bit>  // For std::bit_cast

namespace resource {

	TextureData TextureResource::loadTexture(std::string_view path)
	{
		resource::TextureData textureData;

		// Open the file in binary mode
		std::ifstream inFile(path.data(), std::ios::binary);
		if (!inFile) {
			vfLogError("Failed to open file for reading: ", path);
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

		size_t bytesRemaining = textureDataSize;

		textureData.textureData.resize(textureDataSize);  // Reserve space in advance if possible
		size_t currentOffset = 0;

		while (bytesRemaining > 0) {
			size_t bytesToRead = std::min(chunkSize, bytesRemaining);

			// Read chunk into memory
			inFile.read(std::bit_cast<char*>(&textureData.textureData[currentOffset]), bytesToRead);

			currentOffset += bytesToRead;
			bytesRemaining -= bytesToRead;
		}

		// Close the file
		inFile.close();

		return textureData;
	}
}

