#include "TextureResource.hpp"
#include "../print/EditorLogger.hpp"
#include "HdrHalper.hpp"

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
			return {}; // Return an empty TextureData on failure
		}

		// Read version
		// Read the header file type
		uint8_t headerFileType;
		inFile.read(std::bit_cast<char*>(&headerFileType), sizeof(headerFileType));
		textureData.headerFileType = static_cast<resource::FileType>(headerFileType);

		// Read version
		// Read the version information
		uint32_t majorVersion, minorVersion, patchVersion;
		inFile.read(std::bit_cast<char*>(&majorVersion), sizeof(majorVersion));
		inFile.read(std::bit_cast<char*>(&minorVersion), sizeof(minorVersion));
		inFile.read(std::bit_cast<char*>(&patchVersion), sizeof(patchVersion));

		// Validate the version
		if (majorVersion != Version::major || minorVersion != Version::minor || patchVersion != Version::patch) {
			vfLogError("Incompatible file version: {}.{}.{}", majorVersion, minorVersion, patchVersion);
			return {}; // Return an empty HDRData on version mismatch
		}

		// Read width and height
		inFile.read(std::bit_cast<char*>(&textureData.width), sizeof(textureData.width));
		inFile.read(std::bit_cast<char*>(&textureData.height), sizeof(textureData.height));
		inFile.read(std::bit_cast<char*>(&textureData.numbersOfChannels), sizeof(textureData.numbersOfChannels));

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

	HDRData TextureResource::loadHDR(std::string_view path)
	{
		resource::HDRData hdrData;

		// Open the file in binary mode
		std::ifstream inFile(path.data(), std::ios::binary);
		if (!inFile) {
			vfLogError("Failed to open file for reading: ", path);
			return {}; // Return an empty TextureData on failure
		}

		// Read version
		// Read the header file type
		uint8_t headerFileType;
		inFile.read(std::bit_cast<char*>(&headerFileType), sizeof(headerFileType));
		hdrData.headerFileType = static_cast<resource::FileType>(headerFileType);

		// Read version
		// Read the version information
		uint32_t majorVersion, minorVersion, patchVersion;
		inFile.read(std::bit_cast<char*>(&majorVersion), sizeof(majorVersion));
		inFile.read(std::bit_cast<char*>(&minorVersion), sizeof(minorVersion));
		inFile.read(std::bit_cast<char*>(&patchVersion), sizeof(patchVersion));

		// Validate the version
		if (majorVersion != Version::major || minorVersion != Version::minor || patchVersion != Version::patch) {
			vfLogError("Incompatible file version: {}.{}.{}", majorVersion, minorVersion, patchVersion);
			return {}; // Return an empty HDRData on version mismatch
		}

		// Read width and height
		inFile.read(std::bit_cast<char*>(&hdrData.width), sizeof(hdrData.width));
		inFile.read(std::bit_cast<char*>(&hdrData.height), sizeof(hdrData.height));
		inFile.read(std::bit_cast<char*>(&hdrData.numbersOfChannels), sizeof(hdrData.numbersOfChannels));

		// Read the texture data size and then the texture data itself
	/*	size_t textureDataSize = hdrData.width * hdrData.height * hdrData.numbersOfChannels;
		hdrData.textureData.resize(textureDataSize); // Resize the textureData vector

		size_t currentOffset = 0;

		while (textureDataSize > 0) {
			size_t bytesToRead = std::min(chunkSize, textureDataSize);

			// Read chunk into memory
			inFile.read(std::bit_cast<char*>(&hdrData.textureData[currentOffset]), bytesToRead * sizeof(float));

			currentOffset += bytesToRead;
			textureDataSize -= bytesToRead;
		}*/
		
		HDRReader reader;
		reader.readHDR(inFile, hdrData.width, hdrData.height, hdrData.textureData);
		// Close the file
		inFile.close();
		

		return hdrData;
	}
}

