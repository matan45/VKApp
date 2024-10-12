#pragma once

/*
resource::TextureData loadTextureDataFromFile(const std::string& filename) {
	resource::TextureData textureData;

	// Open the file in binary mode
	std::ifstream inFile(filename, std::ios::binary);
	if (!inFile) {
		std::cerr << "Failed to open file for reading: " << filename << std::endl;
		return textureData; // Return an empty TextureData on failure
	}

	// Read version
	inFile.read(reinterpret_cast<char*>(&textureData.version.major), sizeof(textureData.version.major));
	inFile.read(reinterpret_cast<char*>(&textureData.version.minor), sizeof(textureData.version.minor));
	inFile.read(reinterpret_cast<char*>(&textureData.version.patch), sizeof(textureData.version.patch));

	// Read width and height
	inFile.read(reinterpret_cast<char*>(&textureData.width), sizeof(textureData.width));
	inFile.read(reinterpret_cast<char*>(&textureData.height), sizeof(textureData.height));

	// Read the texture format (length and string)
	uint32_t formatLength;
	inFile.read(reinterpret_cast<char*>(&formatLength), sizeof(formatLength)); // Read the length of the format string
	textureData.textureFormat.resize(formatLength);                            // Resize the string buffer
	inFile.read(&textureData.textureFormat[0], formatLength);                  // Read the format string itself

	// Read the texture data size and then the texture data itself
	uint32_t textureDataSize;
	inFile.read(reinterpret_cast<char*>(&textureDataSize), sizeof(textureDataSize)); // Read the size of the texture data
	textureData.textureData.resize(textureDataSize);                                  // Resize the textureData vector
	inFile.read(reinterpret_cast<char*>(textureData.textureData.data()), textureDataSize); // Read the raw texture data

	// Close the file
	inFile.close();

	std::cout << "Texture data successfully loaded from " << filename << std::endl;
	return textureData;
}
*/
class TextureResource
{
};

