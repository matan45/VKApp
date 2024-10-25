#include "FileUtils.hpp"
#include "../string/StringUtil.hpp"
#include "../print/EditorLogger.hpp"
#include <filesystem>
#include <fstream>
#include <bit>  // For std::bit_cast

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

    std::string FileUtils::getImageFileType(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 10); // Read the first 10 bytes
        if (header.empty())
        {
            return "Unknown"; // Could not read file or file is empty
        }
        if (isPNG(header))
        {
            return "PNG";
        }
        if (isJPG(header))
        {
            return "JPEG";
        }
        if (isBMP(header))
        {
            return "Bitmap";
        }
        if (isHDR(header))
        {
            return "HDR";
        }
        if (isEXR(header))
        {
            return "EXR";
        }
        return "Unknown";
    }

    std::string FileUtils::getAudioFileType(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 12); // Read the first 12 bytes

        if (isMP3(header))
        {
            return "MP3";
        }
        if (isWAV(header))
        {
            return "WAVE";
        }
        if (isOGG(header))
        {
            return "OGG";
        }
        return "Unknown";
    }

    std::string FileUtils::getMeshFileType(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 10); // Read the first 10 bytes

        if (isOBJ(header))
        {
            return "OBJ";
        }
        if (isFBX(header))
        {
            return "FBX";
        }
        if (isDAE(header))
        {
            return "DAE";
        }
        if (isGLTF(header))
        {
            return "GLTF";
        }

        return "Unknown";
    }

    bool FileUtils::isMeshFile(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 10); // Read the first 10 bytes

        if (isOBJ(header))
        {
            return true;
        }
        if (isFBX(header))
        {
            return true;
        }
        if (isDAE(header))
        {
            return true;
        }
        if (isGLTF(header))
        {
            return true;
        }

        return false;
    }

    bool FileUtils::isAudioFile(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 12); // Read the first 12 bytes

        if (isMP3(header))
        {
            return true;
        }
        if (isWAV(header))
        {
            return true;
        }
        if (isOGG(header))
        {
            return true;
        }

        return false;
    }

    std::vector<unsigned char> FileUtils::readHeader(std::string_view filename, size_t numBytes)
    {
        std::vector<unsigned char> header(numBytes);
        std::ifstream file(filename.data(), std::ios::binary);
        if (file.is_open())
        {
            file.read(std::bit_cast<char*>(header.data()), numBytes);
        }
        file.close();
        return header;
    }

    bool FileUtils::isPNG(const std::vector<unsigned char>& header)
    {
        const unsigned char pngSignature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        return header.size() >= 8 && std::equal(header.begin(), header.begin() + 8, pngSignature);
    }

    bool FileUtils::isJPG(const std::vector<unsigned char>& header)
    {
        const unsigned char jpgSignature[3] = {0xFF, 0xD8, 0xFF};
        return header.size() >= 3 && std::equal(header.begin(), header.begin() + 3, jpgSignature);
    }

    bool FileUtils::isBMP(const std::vector<unsigned char>& header)
    {
        const unsigned char bmpSignature[2] = {0x42, 0x4D}; // 'BM'
        return header.size() >= 2 && std::equal(header.begin(), header.begin() + 2, bmpSignature);
    }

    bool FileUtils::isHDR(const std::vector<unsigned char>& header)
    {
        static const std::string hdrSignature1 = "#?RADIANCE";
        static const std::string hdrSignature2 = "#?RGBE";
        return header.size() >= hdrSignature1.size() &&
        (std::equal(header.begin(), header.begin() + hdrSignature1.size(), hdrSignature1.begin()) ||
            std::equal(header.begin(), header.begin() + hdrSignature2.size(), hdrSignature2.begin()));
    }

    bool FileUtils::isEXR(const std::vector<unsigned char>& header)
    {
        const unsigned char exrSignature[4] = {0x76, 0x2F, 0x31, 0x01}; // 'v/1' + 0x01
        return header.size() >= 4 && std::equal(header.begin(), header.begin() + 4, exrSignature);
    }

    bool FileUtils::isMP3(const std::vector<unsigned char>& header)
    {
        // Check for 'ID3' tag at the start or typical MP3 frame markers
        return header.size() >= 3 &&
            (header[0] == 0x49 && header[1] == 0x44 && header[2] == 0x33) || // 'ID3'
            (header[0] == 0xFF && (header[1] & 0xE0) == 0xE0); // Frame sync bytes (FF Fx)
    }

    bool FileUtils::isWAV(const std::vector<unsigned char>& header)
    {
        // Check for 'RIFF' followed by 'WAVE'
        return header.size() >= 12 &&
            header[0] == 0x52 && header[1] == 0x49 && header[2] == 0x46 && header[3] == 0x46 && // 'RIFF'
            header[8] == 0x57 && header[9] == 0x41 && header[10] == 0x56 && header[11] == 0x45; // 'WAVE'
    }

    bool FileUtils::isOGG(const std::vector<unsigned char>& header)
    {
        // Check for 'OggS' signature
        const unsigned char oggSignature[4] = {0x4F, 0x67, 0x67, 0x53}; // 'OggS'
        return header.size() >= 4 && std::equal(header.begin(), header.begin() + 4, oggSignature);
    }

    bool FileUtils::isOBJ(const std::vector<unsigned char>& header)
    {
        // Check for 'Kaydara FBX Binary' signature for binary FBX
        const std::string fbxSignature = "Kaydara FBX Binary  ";
        return header.size() >= fbxSignature.size() &&
            std::equal(fbxSignature.begin(), fbxSignature.end(), header.begin());
    }

    bool FileUtils::isFBX(const std::vector<unsigned char>& header)
    {
        // Check for 'Kaydara FBX Binary' signature for binary FBX
        const std::string fbxSignature = "Kaydara FBX Binary  ";
        return header.size() >= fbxSignature.size() &&
            std::equal(fbxSignature.begin(), fbxSignature.end(), header.begin());
    }

    bool FileUtils::isDAE(const std::vector<unsigned char>& header)
    {
        // Check for '<?xml' in the first few bytes for XML-based DAE
        const std::string xmlSignature = "<?xml";
        const std::string colladaTag = "<COLLADA";
        return header.size() >= xmlSignature.size() &&
            std::equal(xmlSignature.begin(), xmlSignature.end(),
                       header.begin() + StringUtil::findFirstNotOf(header, "\n\r\t ")) &&
            std::search(header.begin(), header.end(), colladaTag.begin(), colladaTag.end()) != header.end();
    }

    bool FileUtils::isGLTF(const std::vector<unsigned char>& header)
    {
        // Check for '{' for JSON-based GLTF or 'glTF' for binary GLB
        const std::string glbSignature = "glTF";
        return header.size() >= 1 && header[0] == '{' ||
            (header.size() >= 4 && std::equal(glbSignature.begin(), glbSignature.end(), header.begin()));
    }

    bool FileUtils::isTextureFile(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 10); // Read the first 10 bytes

        if (isPNG(header))
        {
            return true;
        }
        if (isJPG(header))
        {
            return true;
        }
        if (isBMP(header))
        {
            return true;
        }

        return false;
    }

    bool FileUtils::isHDRFile(std::string_view filePath)
    {
        std::vector<unsigned char> header = readHeader(filePath, 10); // Read the first 10 bytes
        if (isHDR(header))
        {
            return true;
        }
        if (isEXR(header))
        {
            return true;
        }

        return false;
    }
}
