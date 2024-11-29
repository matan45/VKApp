#include "Texture.hpp"
#include "print/EditorLogger.hpp"
#include "../controllers/files/FileUtils.hpp"
#include "config/Config.hpp"
#include "TGA.hpp"

#include <iostream>
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
#include <bit>
#include <filesystem>


namespace types
{
	void Texture::loadTextureFile(const importConfig::ImportFiles& file, std::string_view fileName,
		std::string_view location)
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

		convertTo4Channels(imageData, width, height, channels, textureData.textureData);

		// Store raw texture data into the vector
		//textureData.textureData = std::vector<unsigned char>(imageData, imageData + (width * height * channels));

		if (file.config.isImageFlipVertically)
		{
			stbi_set_flip_vertically_on_load(false);
		}

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
				return;
			}

			if (file.config.isImageFlipVertically)
			{
				stbi_set_flip_vertically_on_load(false);
			}

			hdrData.width = static_cast<uint32_t>(width);
			hdrData.height = static_cast<uint32_t>(height);
			hdrData.numbersOfChannels = channels;
			hdrData.textureData = std::vector<float>(imageData, imageData + (width * height * channels));

			saveToFileHDR(fileName, location, hdrData);

			stbi_image_free(imageData);
		}
		else if (type == "EXR")
		{
			EXRVersion exrVersion;

			int ret = ParseEXRVersionFromFile(&exrVersion, file.path.data());
			if (ret != TINYEXR_SUCCESS)
			{
				vfLogError("Invalid EXR file: {}", file.path.data());
				return;
			}

			EXRHeader exrHeader;
			InitEXRHeader(&exrHeader);

			const char* exrError = nullptr;
			ret = ParseEXRHeaderFromFile(&exrHeader, &exrVersion, file.path.data(), &exrError);
			if (ret != TINYEXR_SUCCESS)
			{
				vfLogError("Parse EXR err: {}", exrError);
				FreeEXRErrorMessage(exrError);
				return;
			}

			EXRImage exrImage;
			InitEXRImage(&exrImage);

			ret = LoadEXRImageFromFile(&exrImage, &exrHeader, file.path.data(), &exrError);
			if (ret != TINYEXR_SUCCESS)
			{
				vfLogError("Load EXR err: {}", exrError);
				FreeEXRHeader(&exrHeader);
				FreeEXRErrorMessage(exrError);
				return;
			}

			float* out;
			int width;
			int height;

			int result = LoadEXR(&out, &width, &height, file.path.data(), &exrError);
			if (result != TINYEXR_SUCCESS)
			{
				vfLogError("Failed to load EXR image: {}", exrError);
				FreeEXRErrorMessage(exrError);
				return;
			}

			if (file.config.isImageFlipVertically)
			{
				flipImageVertically(out, width, height);
			}

			int channels = exrImage.num_channels < 4 ? exrImage.num_channels : 4;

			hdrData.width = static_cast<uint32_t>(width);
			hdrData.height = static_cast<uint32_t>(height);
			hdrData.numbersOfChannels = static_cast<uint32_t>(channels);
			hdrData.textureData = convertFromEXRToHDR(out, width, height);

			free(out);
			FreeEXRImage(&exrImage);
			FreeEXRHeader(&exrHeader);

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
		outFile.write(std::bit_cast<const char*>(&textureData.numbersOfChannels),
			sizeof(textureData.numbersOfChannels));

		TGAWriter::writeTGA(outFile, textureData.textureData);

		// Close the file
		outFile.close();
	}

	void Texture::saveToFileHDR(std::string_view fileName, std::string_view location,
		const resource::HDRData& hdrData) const
	{
		std::filesystem::path newFileLocation = std::filesystem::path(location) / (std::string(fileName) + "." +
			FileExtension::hdr);
		std::ofstream outFile(newFileLocation, std::ios::binary);

		if (!outFile)
		{
			vfLogError("Failed to open file for writing: ", newFileLocation.string());
			return;
		}

		uint8_t headerFileType = static_cast<uint8_t>(hdrData.headerFileType);
		outFile.write(reinterpret_cast<const char*>(&headerFileType), sizeof(headerFileType));

		uint32_t majorVersion = std::bit_cast<uint32_t>(Version::major);
		uint32_t minorVersion = std::bit_cast<uint32_t>(Version::minor);
		uint32_t patchVersion = std::bit_cast<uint32_t>(Version::patch);
		outFile.write(std::bit_cast<const char*>(&majorVersion), sizeof(majorVersion));
		outFile.write(std::bit_cast<const char*>(&minorVersion), sizeof(minorVersion));
		outFile.write(std::bit_cast<const char*>(&patchVersion), sizeof(patchVersion));

		outFile.write(std::bit_cast<const char*>(&hdrData.width), sizeof(hdrData.width));
		outFile.write(std::bit_cast<const char*>(&hdrData.height), sizeof(hdrData.height));
		outFile.write(std::bit_cast<const char*>(&hdrData.numbersOfChannels), sizeof(hdrData.numbersOfChannels));

		HDRWriter::writeHDR(outFile, hdrData.width, hdrData.height, hdrData.numbersOfChannels, hdrData.textureData);

		outFile.close();
	}

	void Texture::convertTo4Channels(unsigned char* inputData, int width, int height, int inputChannels, std::vector<unsigned char>& outputData)
	{
		int outputChannels = 4; // RGBA
		outputData.resize(width * height * outputChannels);

		for (int i = 0; i < width * height; ++i) {
			unsigned char r, g, b, a;
			if (inputChannels == 1) {
				// Grayscale -> RGBA
				r = g = b = inputData[i];
				a = 255; // Default alpha
			}
			else if (inputChannels == 2) {
				// Grayscale + Alpha -> RGBA
				r = g = b = inputData[i * 2];
				a = inputData[i * 2 + 1];
			}
			else if (inputChannels == 3) {
				// RGB -> RGBA
				r = inputData[i * 3];
				g = inputData[i * 3 + 1];
				b = inputData[i * 3 + 2];
				a = 255; // Default alpha
			}
			else if (inputChannels == 4) {
				// Already RGBA
				r = inputData[i * 4];
				g = inputData[i * 4 + 1];
				b = inputData[i * 4 + 2];
				a = inputData[i * 4 + 3];
			}
			else {
				//throw std::runtime_error("Unsupported number of channels");
			}

			outputData[i * 4] = r;
			outputData[i * 4 + 1] = g;
			outputData[i * 4 + 2] = b;
			outputData[i * 4 + 3] = a;
		}
	}

	std::vector<float> Texture::convertFromEXRToHDR(const float* data, int width, int height) const
	{
		std::vector<float> result(width * height * 3); // RGB needs 3 floats per pixel
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				const int index = (y * width + x) * 4; // EXR data has 4 channels (RGBA)
				float r = data[index];
				float g = data[index + 1];
				float b = data[index + 2];

				// Normalize RGB values to [0, 1]
				float maxValue = std::max({ r, g, b, 1e-6f }); // Avoid division by zero
				if (maxValue > 1.0f)
				{
					r /= maxValue;
					g /= maxValue;
					b /= maxValue;
				}

				// Store normalized values in result
				int resultIndex = (y * width + x) * 3;
				result[resultIndex] = r;
				result[resultIndex + 1] = g;
				result[resultIndex + 2] = b;
			}
		}
		return result;
	}

	void Texture::flipImageVertically(float* imageData, int width, int height) const
	{
		if (!imageData || width <= 0 || height <= 0)
		{
			return; // Invalid input
		}

		int rowSize = width * 4; // Number of floats per row (RGBA)
		float* tempRow = new float[rowSize]; // Temporary buffer to hold a row

		for (int y = 0; y < height / 2; ++y)
		{
			// Calculate row indices to swap
			float* topRow = imageData + y * rowSize;
			float* bottomRow = imageData + (height - 1 - y) * rowSize;

			// Swap rows
			std::memcpy(tempRow, topRow, rowSize * sizeof(float));
			std::memcpy(topRow, bottomRow, rowSize * sizeof(float));
			std::memcpy(bottomRow, tempRow, rowSize * sizeof(float));
		}

		delete[] tempRow;
	}

	void HDRWriter::writeHDR(std::ofstream& file, int width, int height, int numbersOfChannels,
		const std::vector<float>& pixels)
	{
		if (pixels.size() != width * height * numbersOfChannels)
		{
			vfLogError("Pixel data size does not match image dimensions!");
			return;
		}
		for (int y = 0; y < height; ++y)
		{
			// Write scanline header
			uint8_t scanlineHeader[4] = { 2, 2, (uint8_t)(width >> 8), (uint8_t)(width & 0xFF) };
			file.write(reinterpret_cast<char*>(scanlineHeader), 4);

			for (int channel = 0; channel < 4; ++channel)
			{
				int x = 0;
				while (x < width)
				{
					int runLength = 1;
					while (x + runLength < width && runLength < 127 &&
						getChannel(pixels, width, y, x, channel) ==
						getChannel(pixels, width, y, x + runLength, channel))
					{
						runLength++;
					}

					if (runLength > 1)
					{
						// RLE
						uint8_t value = getChannel(pixels, width, y, x, channel);
						file.put(128 + runLength);
						file.put(value);
					}
					else
					{
						// Raw data
						uint8_t value = getChannel(pixels, width, y, x, channel);
						file.put(1);
						file.put(value);
					}
					x += runLength;
				}
			}
		}
	}

	uint8_t HDRWriter::getChannel(const std::vector<float>& pixels, int width, int y, int x, int channel)
	{
		float r = pixels[(y * width + x) * 3 + 0];
		float g = pixels[(y * width + x) * 3 + 1];
		float b = pixels[(y * width + x) * 3 + 2];

		switch (channel)
		{
		case 0: return encodeRGBE(r, g, b).r; // Red
		case 1: return encodeRGBE(r, g, b).g; // Green
		case 2: return encodeRGBE(r, g, b).b; // Blue
		case 3: return encodeRGBE(r, g, b).e; // Exponent
		default: return 0;
		}
	}

	RGBE HDRWriter::encodeRGBE(float r, float g, float b)
	{
		float maxColor = std::max(r, std::max(g, b));
		if (maxColor < 1e-5f) return { 0, 0, 0, 0 };

		int e;
		float scale = std::frexp(maxColor, &e) * 256.0f / maxColor;

		return {
			(uint8_t)(r * scale),
			(uint8_t)(g * scale),
			(uint8_t)(b * scale),
			(uint8_t)(e + 128)
		};
	}

	void TGAWriter::writeTGA(std::ofstream& file,const std::vector<unsigned char>& pixelData)
	{

		for (size_t i = 0; i < pixelData.size(); i += 4)
		{
			uint8_t bgra[4] = { pixelData[i + 2], pixelData[i + 1], pixelData[i], pixelData[i + 3] };
			file.write(reinterpret_cast<char*>(bgra), sizeof(bgra));
		}

	}
}
