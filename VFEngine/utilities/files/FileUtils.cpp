#include "FileUtils.hpp"
#include "../string/StringUtil.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace files
{
    std::string FileUtils::getFileExtension(std::string_view filePath, bool lowerCase)
    {
        fs::path fsPath(filePath.data());
        std::string extension = fsPath.extension().string();
        extension.erase(std::ranges::find(extension.begin(), extension.end(), '\0'), extension.end());
        if (lowerCase)
        {
            extension = StringUtil::toLower(extension);
        }
        return extension;
    }

    std::string FileUtils::getFileName(std::string_view filePath)
    {
        fs::path fsPath(filePath.data());
        // Get the file name
        return fsPath.stem().string();
    }

    bool FileUtils::isMeshFile(std::string_view filePath)
    {
        std::string extension = getFileExtension(filePath);
        return extension == ".obj" || extension == ".fbx" || extension == ".dae" || extension == ".gltf";
    }

    bool FileUtils::isAudioFile(std::string_view filePath)
    {
        std::string extension = getFileExtension(filePath);
        return extension == ".wav" || extension == ".mp3" || extension == ".ogg";
    }

    bool FileUtils::isImageFile(std::string_view filePath)
    {
        std::string extension = getFileExtension(filePath);
        return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".hdr";
    }
}
