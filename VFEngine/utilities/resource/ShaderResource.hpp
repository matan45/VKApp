#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <string>

#include "Types.hpp"

namespace resource {

	struct ShaderModel {
		ShaderType type;
		std::string source;

		ShaderModel(ShaderType t, const std::string& s) : type(t), source(s) {}
	};


	class ShaderResource
	{
	public:
		static std::vector<ShaderModel> readShaderFile(std::string_view filePath);
	private:
		static ShaderType getShaderType(std::string_view shaderType);
	};

}

