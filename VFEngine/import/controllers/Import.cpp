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

	void Import::processPath(const std::string& path)
	{
		// Convert the string path to a std::filesystem::path object
		fs::path fsPath(path);

		// Get the file name
		std::string fileName = fsPath.filename().string();
		std::cout << "File name: " << fileName << std::endl;

		// Get the file extension
		std::string extension = fsPath.extension().string();
		std::ranges::transform(extension.begin(), extension.end(), extension.begin(), ::tolower); // Convert to lowercase

		if (extension.empty()) {
			std::cerr << "No file extension found for path: " << path << std::endl;
			return;
		}

		// Get the parent directory (path without file name and extension)
		std::string directoryPath = fsPath.parent_path().string();
		std::cout << "Directory path: " << directoryPath << std::endl;

		// Use if-else to handle different file types
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr") {
			// Handle texture files
			//processTexture(path);
		}
		else if (extension == ".obj" || extension == ".fbx" || extension == ".dae" || extension == ".glTF") {
			// Handle model and animations files
			//processModel(path);
		}
		else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
			// Handle model files
			//processModel(path);
		}
		else {
			std::cerr << "Unknown file extension: " << extension << std::endl;
		}
	}

}
