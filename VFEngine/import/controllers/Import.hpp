#pragma once
#include <vector>
#include <string>
#include "../types/Audio.hpp"
#include "../types/Texture.hpp"
#include "../types/Mesh.hpp"


namespace controllers {
	class Import
	{
	private:
		inline static std::string location;
		inline static types::Audio audio;
		inline static types::Mesh mesh;
		inline static types::Texture texture;
	public:
		static void importFiles(const std::vector<std::string>& paths);
		static void setLocation(const std::string& newLocation);

	private:
		static void processPath(const std::string& path); 
		static void processTexture(std::string_view path, std::string_view fileName, std::string_view location);
		static void processModel(std::string_view path, std::string_view fileName, std::string_view extension, std::string_view location);
		static void processAudio(std::string_view path, std::string_view fileName, std::string_view extension, std::string_view location);
	};
}


