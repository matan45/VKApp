#include "Import.hpp"
#include <filesystem> 
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace controllers {

	void Import::importFiles(const std::vector<std::string>& paths)
	{
		for (const auto& path : paths) {
			//TODO should be anysc
			processPath(path);
		}
	}

	void Import::setLocation(const std::string& newLocation)
	{
		location = newLocation;
	}

	void Import::processPath(const std::string& path)
	{
		// Convert the string path to a std::filesystem::path object
		fs::path fsPath(path);

		// Get the file name
		std::string fileName = fsPath.stem().string();
		std::cout << "File name: " << fileName << std::endl;

		// Get the file extension
		std::string extension = fsPath.extension().string();
		extension.erase(std::find(extension.begin(), extension.end(), '\0'), extension.end());
		std::ranges::transform(extension.begin(), extension.end(), extension.begin(), ::tolower); // Convert to lowercase

		if (extension.empty()) {
			std::cerr << "No file extension found for path: " << path << std::endl;
			return;
		}

		// Use if-else to handle different file types
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr") {
			// Handle texture files
			processTexture(path, fileName, location);
		}
		else if (extension == ".obj" || extension == ".fbx" || extension == ".dae" || extension == ".glTF") {
			// Handle model and animations files
			processModel(path, fileName, location);
		}
		else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
			// Handle model files
			processAudio(path, fileName, extension, location);
		}
		else {
			std::cerr << "Unknown file extension: " << extension << std::endl;
		}
	}

	void Import::processTexture(std::string_view path, std::string_view fileName, std::string_view location)
	{
		texture.loadFromFile(path, fileName, location);
	}

	void Import::processModel(std::string_view path, std::string_view fileName, std::string_view location)
	{
		mesh.loadFromFile(path, fileName, location);
	}

	void Import::processAudio(std::string_view path, std::string_view fileName, std::string_view extension, std::string_view location)
	{
		audio.loadFromFile(path, fileName, extension, location);
	}
}
