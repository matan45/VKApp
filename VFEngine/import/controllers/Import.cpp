#include "Import.hpp"
#include <filesystem> 
#include <algorithm>
#include <future>

#include "print/EditorLogger.hpp"
#include "string/StringUtil.hpp"


namespace fs = std::filesystem;

namespace controllers {

	void Import::importFiles(const std::vector<std::string>& paths)
	{
		std::vector<std::future<void>> futures;
		for (const auto& path : paths) {
			futures.push_back(std::async(std::launch::async, &Import::processPath, path));
		}
	}

	void Import::setLocation(std::string_view newLocation)
	{
		location = newLocation;
	}

	void Import::processPath(const std::string& path)
	{
		// Convert the string path to a std::filesystem::path object
		fs::path fsPath(path);

		// Get the file name
		std::string fileName = fsPath.stem().string();
		vfLogInfo("Import File name: {}", fileName);

		// Get the file extension
		std::string extension = fsPath.extension().string();
		extension.erase(std::find(extension.begin(), extension.end(), '\0'), extension.end());
		extension = StringUtil::toLower(extension);

		if (extension.empty()) {
			vfLogError("No file extension found for path: {}", path);
			return;
		}

		// Use if-else to handle different file types
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr") {
			// Handle texture files
			processTexture(path, fileName);
		}
		else if (extension == ".obj" || extension == ".fbx" || extension == ".dae" || extension == ".gltf") {
			// Handle model and animations files
			processModel(path, fileName);
		}
		else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
			// Handle model files
			processAudio(path, fileName, extension);
		}
		else {
			vfLogError("Unknown file extension: {}", extension);
		}
	}

	void Import::processTexture(std::string_view path, std::string_view fileName)
	{
		texture.loadFromFile(path, fileName, location);
	}

	void Import::processModel(std::string_view path, std::string_view fileName)
	{
		mesh.loadFromFile(path, fileName, location);
	}

	void Import::processAudio(std::string_view path, std::string_view fileName, std::string_view extension)
	{
		audio.loadFromFile(path, fileName, extension, location);
	}
}
