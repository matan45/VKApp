#include "Texture.hpp"
#include "print/EditorLogger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <vector>
#include <fstream>
#include <bit>  // For std::bit_cast
#include <stb_image.h>
#include <filesystem> 

#include "config/Config.hpp"

namespace types {
	void Texture::loadTextureFile(const importConfig::ImportFiles& file, std::string_view fileName, std::string_view location) const
	{
		resource::TextureData textureData;

		if(file.config.isImageFlipVertically)
		{
			stbi_set_flip_vertically_on_load(true);
		}
		// Load image using stb_image
		int width;
		int height;
		int channels;
		//TODO load float for hdr stbi_loadf
		unsigned char* imageData = stbi_load(file.path.data(), &width, &height, &channels, 0);

		if (!imageData) {
			vfLogError("Failed to load texture: {}", file.path.data());
			return; // Return
		}

		// Store texture information
		textureData.width = static_cast<uint32_t>(width);
		textureData.height = static_cast<uint32_t>(height);

		// Determine texture format based on channels
		switch (channels) {
		case 1: textureData.textureFormat = "R"; break;
		case 3: textureData.textureFormat = "RGB"; break;
		case 4: textureData.textureFormat = "RGBA"; break;
		default: textureData.textureFormat = "Unknown"; break;
		}

		// Store raw texture data into the vector
		textureData.textureData = std::vector<unsigned char>(imageData, imageData + (width * height * channels));

		if(file.config.isImageFlipVertically)
		{
			stbi_set_flip_vertically_on_load(false);
		}
		// Free the image data once copied to the structure
		stbi_image_free(imageData);

		saveToFileTexture(fileName, location, textureData);
	}

	void Texture::loadHDRFile(const importConfig::ImportFiles& file, std::string_view fileName,
		std::string_view location) const
	{
	}

	void Texture::saveToFileTexture(std::string_view fileName, std::string_view location, const resource::TextureData& textureData) const
	{
		// Open the file in binary mode
		std::filesystem::path newFileLocation = std::filesystem::path(location) / (std::string(fileName) + "." + FileExtension::textrue);
		std::ofstream outFile(newFileLocation, std::ios::binary);

		if (!outFile) {
			vfLogError("Failed to open file for writing: ",newFileLocation.string());
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
	}

	void Texture::saveToFileHDR(std::string_view fileName, std::string_view location,
		const resource::HDRData& hdrData) const
	{
	}
	
}
