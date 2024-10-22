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
		static void setLocation(std::string_view newLocation);

	private:
		static void processPath(const std::string& path); 
		static void processTexture(std::string_view path, std::string_view fileName);
		static void processModel(std::string_view path, std::string_view fileName);
		static void processAudio(std::string_view path, std::string_view fileName, std::string_view extension);
	};
}


