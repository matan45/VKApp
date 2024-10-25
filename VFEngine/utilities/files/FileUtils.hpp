#pragma once
#include <string>

namespace files
{
    class FileUtils
    {
    public:
        static std::string getFileExtension(std::string_view filePath,bool lowerCase = true);
        static std::string getFileName(std::string_view filePath);
        static bool isImageFile(std::string_view filePath);
        static bool isMeshFile(std::string_view filePath);
        static bool isAudioFile(std::string_view filePath);
    };
}

