#pragma once
#include <string>
#include <vector>

namespace files
{
    //TODO move it to import
    //change the ib name to commons
    class FileUtils
    {
    public:
        static std::string getFileExtension(std::string_view filePath, bool lowerCase = true);
        static std::string getFileName(std::string_view filePath);

        static std::string getImageFileType(std::string_view filePath);
        static std::string getAudioFileType(std::string_view filePath);
        static std::string getMeshFileType(std::string_view filePath);

        static bool isTextureFile(std::string_view filePath);
        static bool isHDRFile(std::string_view filePath);
        static bool isMeshFile(std::string_view filePath);
        static bool isAudioFile(std::string_view filePath);

    private:
        static std::vector<unsigned char> readHeader(std::string_view filename, size_t numBytes);
        static bool isPNG(const std::vector<unsigned char>& header);
        static bool isJPG(const std::vector<unsigned char>& header);
        static bool isBMP(const std::vector<unsigned char>& header);
        static bool isHDR(const std::vector<unsigned char>& header);
        static bool isEXR(const std::vector<unsigned char>& header);

        static bool isMP3(const std::vector<unsigned char>& header);
        static bool isWAV(const std::vector<unsigned char>& header);
        static bool isOGG(const std::vector<unsigned char>& header);

        static bool isOBJ(const std::vector<unsigned char>& header);
        static bool isFBX(const std::vector<unsigned char>& header);
        static bool isDAE(const std::vector<unsigned char>& header);
        static bool isGLTF(const std::vector<unsigned char>& header);
    };
}
