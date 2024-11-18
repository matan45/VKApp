#include "Texture.hpp"
#include "print/EditorLogger.hpp"
#include "../controllers/files/FileUtils.hpp"

#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <vector>
#include <fstream>
#include <bit>  // For std::bit_cast

#include <filesystem>


#include "config/Config.hpp"

namespace types
{
	void Texture::loadTextureFile(const importConfig::ImportFiles& file, std::string_view fileName,
		std::string_view location) const
	{
		resource::TextureData textureData;
		textureData.headerFileType = resource::FileType::TEXTURE;

		if (file.config.isImageFlipVertically)
		{
			stbi_set_flip_vertically_on_load(true);
		}
		// Load image using stb_image
		int width;
		int height;
		int channels;
		unsigned char* imageData = stbi_load(file.path.data(), &width, &height, &channels, 0);

		if (!imageData)
		{
			vfLogError("Failed to load texture: {}", file.path.data());
			return; // Return
		}

		// Store texture information
		textureData.width = static_cast<uint32_t>(width);
		textureData.height = static_cast<uint32_t>(height);
		textureData.numbersOfChannels = channels;

		// Store raw texture data into the vector
		textureData.textureData = std::vector<unsigned char>(imageData, imageData + (width * height * channels));

		if (file.config.isImageFlipVertically)
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
		resource::HDRData hdrData;
		hdrData.headerFileType = resource::FileType::HDR;
		std::string type = files::FileUtils::getImageFileType(file.path.data());
		if (type == "HDR")
		{
			if (file.config.isImageFlipVertically)
			{
				stbi_set_flip_vertically_on_load(true);
			}
			// Load image using stb_image
			int width;
			int height;
			int channels;
			float* imageData = stbi_loadf(file.path.data(), &width, &height, &channels, 0);

			if (!imageData)
			{
				vfLogError("Failed to load texture: {}", file.path.data());
				return; // Return
			}

			// Store texture information
			hdrData.width = static_cast<uint32_t>(width);
			hdrData.height = static_cast<uint32_t>(height);
			hdrData.numbersOfChannels = channels;

			// Store raw texture data into the vector
			hdrData.textureData = std::vector<float>(imageData, imageData + (width * height * channels));

			if (file.config.isImageFlipVertically)
			{
				stbi_set_flip_vertically_on_load(false);
			}
			saveToFileHDR(fileName, location, hdrData);

			stbi_image_free(imageData);
		}
		else if (type == "EXR")
		{
			EXRVersion exrVersion;

			int ret = ParseEXRVersionFromFile(&exrVersion, file.path.data());
			if (ret != TINYEXR_SUCCESS) {
				vfLogError("Invalid EXR file: {}", file.path.data());
				return;
			}

			EXRHeader exrHeader;
			InitEXRHeader(&exrHeader);

			const char* exrError = nullptr;
			ret = ParseEXRHeaderFromFile(&exrHeader, &exrVersion, file.path.data(), &exrError);
			if (ret != TINYEXR_SUCCESS) {
				vfLogError("Parse EXR err: {}", exrError);
				FreeEXRErrorMessage(exrError); // free's buffer for an error message
				return;
			}

			EXRImage exrImage;
			InitEXRImage(&exrImage);

			ret = LoadEXRImageFromFile(&exrImage, &exrHeader, file.path.data(), &exrError);
			if (ret != TINYEXR_SUCCESS) {
				vfLogError("Load EXR err: {}", exrError);
				FreeEXRHeader(&exrHeader);
				FreeEXRErrorMessage(exrError); // free's buffer for an error message
				return;
			}

			float* out; // width * height * RGBA
			int width;
			int height;

			int result = LoadEXR(&out, &width, &height, file.path.data(), &exrError);
			if (result != TINYEXR_SUCCESS)
			{
				vfLogError("Failed to load EXR image: {}", exrError);
				FreeEXRErrorMessage(exrError);
				return;
			}

			int channels = exrImage.num_channels < 4 ? exrImage.num_channels : 4;

			// Allocate space for texture data
			hdrData.width = static_cast<uint32_t>(width);
			hdrData.height = static_cast<uint32_t>(height);
			hdrData.numbersOfChannels = static_cast<uint32_t>(channels);
			hdrData.textureData.resize(width * height * channels);

			// Copy the data into the hdrData.textureData
			std::copy_n(out, width * height * channels, hdrData.textureData.begin());
			// If vertical flip is enabled, flip the image data
			if (file.config.isImageFlipVertically) {
				for (int y = 0; y < height / 2; ++y) {
					int oppositeY = height - y - 1;
					for (int x = 0; x < width * channels; ++x) {
						std::swap(
							hdrData.textureData[y * width * channels + x],
							hdrData.textureData[oppositeY * width * channels + x]
						);
					}
				}
			}

			// Free the memory allocated by LoadEXR
			free(out);

			FreeEXRImage(&exrImage);
			FreeEXRHeader(&exrHeader);

			// Save the EXR data to a file
			saveToFileHDR(fileName, location, hdrData);
		}
	}

	void Texture::saveToFileTexture(std::string_view fileName, std::string_view location,
		const resource::TextureData& textureData) const
	{
		// Open the file in binary mode
		std::filesystem::path newFileLocation = std::filesystem::path(location) / (std::string(fileName) + "." +
			FileExtension::textrue);
		std::ofstream outFile(newFileLocation, std::ios::binary);

		if (!outFile)
		{
			vfLogError("Failed to open file for writing: ", newFileLocation.string());
			return;
		}

		// Write the header file type
		uint8_t headerFileType = static_cast<uint8_t>(textureData.headerFileType);
		outFile.write(std::bit_cast<const char*>(&headerFileType), sizeof(headerFileType));

		// Serialize the mesh data (this is just an example, adapt to your format)
		uint32_t majorVersion = std::bit_cast<uint32_t>(Version::major);
		uint32_t minorVersion = std::bit_cast<uint32_t>(Version::minor);
		uint32_t patchVersion = std::bit_cast<uint32_t>(Version::patch);
		outFile.write(std::bit_cast<const char*>(&majorVersion), sizeof(majorVersion));
		outFile.write(std::bit_cast<const char*>(&minorVersion), sizeof(minorVersion));
		outFile.write(std::bit_cast<const char*>(&patchVersion), sizeof(patchVersion));

		// Write width and height
		outFile.write(std::bit_cast<const char*>(&textureData.width), sizeof(textureData.width));
		outFile.write(std::bit_cast<const char*>(&textureData.height), sizeof(textureData.height));
		outFile.write(std::bit_cast<const char*>(&textureData.numbersOfChannels), sizeof(textureData.numbersOfChannels));

		// Write the size of the texture data and then the raw texture data
		auto textureDataSize = static_cast<uint32_t>(textureData.textureData.size());
		outFile.write(std::bit_cast<const char*>(&textureDataSize), sizeof(textureDataSize));
		// Write the size of the texture data
		outFile.write(std::bit_cast<const char*>(textureData.textureData.data()), textureDataSize);
		// Write the raw texture data

		// Close the file
		outFile.close();
	}

	void Texture::saveToFileHDR(std::string_view fileName, std::string_view location,
		const resource::HDRData& hdrData) const
	{
		// Open the file in binary mode
		std::filesystem::path newFileLocation = std::filesystem::path(location) / (std::string(fileName) + "." +
			FileExtension::hdr);
		std::ofstream outFile(newFileLocation, std::ios::binary);

		if (!outFile)
		{
			vfLogError("Failed to open file for writing: ", newFileLocation.string());
			return;
		}

		// Write the header file type
		uint8_t headerFileType = static_cast<uint8_t>(hdrData.headerFileType);
		outFile.write(reinterpret_cast<const char*>(&headerFileType), sizeof(headerFileType));

		// Serialize the mesh data (this is just an example, adapt to your format)
		uint32_t majorVersion = std::bit_cast<uint32_t>(Version::major);
		uint32_t minorVersion = std::bit_cast<uint32_t>(Version::minor);
		uint32_t patchVersion = std::bit_cast<uint32_t>(Version::patch);
		outFile.write(std::bit_cast<const char*>(&majorVersion), sizeof(majorVersion));
		outFile.write(std::bit_cast<const char*>(&minorVersion), sizeof(minorVersion));
		outFile.write(std::bit_cast<const char*>(&patchVersion), sizeof(patchVersion));

		// Write width and height
		outFile.write(std::bit_cast<const char*>(&hdrData.width), sizeof(hdrData.width));
		outFile.write(std::bit_cast<const char*>(&hdrData.height), sizeof(hdrData.height));
		outFile.write(std::bit_cast<const char*>(&hdrData.numbersOfChannels), sizeof(hdrData.numbersOfChannels));

		// Write the size of the texture data and then the raw texture data
		auto textureDataSize = hdrData.width * hdrData.height * hdrData.numbersOfChannels;
		outFile.write(std::bit_cast<const char*>(&textureDataSize), sizeof(textureDataSize));
		// Write the size of the texture data
		outFile.write(std::bit_cast<const char*>(hdrData.textureData.data()), textureDataSize * sizeof(float));
		// Write the raw texture data

		// Close the file
		outFile.close();
	}
}
