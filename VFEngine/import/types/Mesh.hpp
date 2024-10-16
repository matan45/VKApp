#pragma once
#include <string>
#include "resource/Types.hpp"


namespace types {
	class Mesh
	{
	public:
		void loadFromFile(std::string_view path, std::string_view fileName, std::string_view extension, std::string_view location) const;

	private:
		
		void saveToFile(std::string_view location, std::string_view fileName, const resource::Mesh& audioData) const;
	};

}


