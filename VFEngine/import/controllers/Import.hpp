#pragma once
#include <vector>
#include <string>
#include "config/Config.hpp"
#include "../types/Audio.hpp"
#include "../types/Texture.hpp"
#include "../types/Mesh.hpp"


namespace controllers
{
    class Import
    {
    private:
        inline static std::string location;
        inline static types::Audio audio;
        inline static types::Mesh mesh;
        inline static types::Texture texture;

    public:
        static void importFiles(const std::vector<importConfig::ImportFiles>& paths);
        static void setLocation(std::string_view newLocation);

    private:
        static void processPath(const importConfig::ImportFiles& file);
        static void processTexture(const importConfig::ImportFiles& file, std::string_view fileName);
        static void processHDR(const importConfig::ImportFiles& file, std::string_view fileName);
        static void processModel(const importConfig::ImportFiles& file, std::string_view fileName);
        static void processAudio(const importConfig::ImportFiles& file, std::string_view fileName);
    };
}
