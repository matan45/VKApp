#include "Import.hpp"
#include <filesystem> 
#include <future>

#include "files/FileUtils.hpp"
#include "print/EditorLogger.hpp"


namespace controllers {

	void Import::importFiles(const std::vector<importConfig::ImportFiles>& paths)
	{
		std::vector<std::future<void>> futures;
		futures.reserve(paths.size());
		
		for (const auto& path : paths) {
			futures.push_back(std::async(std::launch::async, &Import::processPath, path));
		}
	}

	void Import::setLocation(std::string_view newLocation)
	{
		location = newLocation;
	}

	void Import::processPath(const importConfig::ImportFiles& file)
	{
		// Get the file name
		std::string fileName = files::FileUtils::getFileName(file.path.data());
		vfLogInfo("Import File name: {}", fileName);

		// Use if-else to handle different file types
		if (files::FileUtils::isImageFile(file.path.data())) {
			// Handle texture files
			processTexture(file, fileName);
		}
		else if (files::FileUtils::isMeshFile(file.path.data())) {
			// Handle model and animations files
			processModel(file, fileName);
		}
		else if (files::FileUtils::isAudioFile(file.path.data())) {
			// Handle model files
			processAudio(file, fileName, files::FileUtils::getFileExtension(file.path.data()));
		}
		else {
			vfLogError("Unknown file extension: {}", files::FileUtils::getFileExtension(file.path.data()));
		}
	}

	void Import::processTexture(const importConfig::ImportFiles& file, std::string_view fileName)
	{
		texture.loadFromFile(file, fileName, location);
	}

	void Import::processModel(const importConfig::ImportFiles& file, std::string_view fileName)
	{
		mesh.loadFromFile(file, fileName, location);
	}

	void Import::processAudio(const importConfig::ImportFiles& file, std::string_view fileName, std::string_view extension)
	{
		audio.loadFromFile(file, fileName, extension, location);
	}
}
