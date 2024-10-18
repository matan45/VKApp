#include "ShaderResource.hpp"


namespace resource {
	std::vector<ShaderModel> ShaderResource::readShaderFile(std::string_view filePath)
	{
		std::vector<ShaderModel> shaderModels;
		std::ifstream file(filePath.data());

		if (!file.is_open()) {
			//throw std::runtime_error("Failed to open shader file: " + filePath);
		}

		std::string line;
		std::string shaderSource;
		ShaderType currentType = ShaderType::UNKNOWN;
		const std::string TYPE = "#type ";

		while (std::getline(file, line)) {
			if (line.rfind(TYPE, 0) == 0) {  // Check if line starts with "#type"
				// Save previous shader block before switching to new type
				if (currentType != ShaderType::UNKNOWN && !shaderSource.empty()) {
					shaderModels.emplace_back(currentType, shaderSource);
					shaderSource.clear(); // Reset shader source
				}

				// Extract shader type from the line
				std::string shaderType = line.substr(TYPE.length());
				currentType = getShaderType(shaderType);

				if (currentType == ShaderType::UNKNOWN) {
					throw std::runtime_error("Invalid shader type: " + shaderType);
				}
				continue; // Skip adding the "#type" line to the shader source
			}

			shaderSource += line + "\n"; // Append line to shader source
		}

		// Add the last shader block if exists
		if (currentType != ShaderType::UNKNOWN && !shaderSource.empty()) {
			shaderModels.emplace_back(currentType, shaderSource);
		}

		return shaderModels;
	}


	ShaderType ShaderResource::getShaderType(std::string_view shaderType)
	{
		if (shaderType == "VERTEX") return ShaderType::VERTEX;
		if (shaderType == "FRAGMENT") return ShaderType::FRAGMENT;
		if (shaderType == "COMPUTE") return ShaderType::COMPUTE;
		if (shaderType == "GEOMETRY") return ShaderType::GEOMETRY;
		if (shaderType == "CONTROL") return ShaderType::TESS_CONTROL;
		if (shaderType == "EVALUATION") return ShaderType::TESS_EVALUATION;
		return ShaderType::UNKNOWN;
	}
}

