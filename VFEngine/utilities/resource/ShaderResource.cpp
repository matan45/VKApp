#include "ShaderResource.hpp"
#include "../print/EditorLogger.hpp"


namespace resource {
	std::vector<ShaderModel> ShaderResource::readShaderFile(std::string_view filePath)
	{
		std::vector<ShaderModel> shaderModels;
		std::ifstream file(filePath.data());

		if (!file.is_open()) {
			vfLogError("Failed to open shader file: {}", filePath);
			return std::vector<ShaderModel>();
		}

		std::string line;
		std::ostringstream shaderStream;
		ShaderType currentType = ShaderType::UNKNOWN;
		const std::string TYPE = "#type ";

		while (std::getline(file, line)) {
			if (line.rfind(TYPE, 0) == 0) {  // Check if line starts with "#type"
				// Save previous shader block before switching to new type
				if (currentType != ShaderType::UNKNOWN && !shaderStream.str().empty()) {
					shaderModels.emplace_back(currentType, shaderStream.str());
					shaderStream.str("");
					shaderStream.clear();
				}

				// Extract shader type from the line
				std::string shaderType = line.substr(TYPE.length());
				currentType = getShaderType(shaderType);

				if (currentType == ShaderType::UNKNOWN) {
					vfLogError("Invalid shader type: {} in file: {}", shaderType, std::string(filePath));
					return std::vector<ShaderModel>();
				}
				continue;
			}

			shaderStream << line << "\n";
		}

		// Add the last shader block if exists
		if (currentType != ShaderType::UNKNOWN && !shaderStream.str().empty()) {
			shaderModels.emplace_back(currentType, shaderStream.str());
		}

		return shaderModels;
	}


	ShaderType ShaderResource::getShaderType(std::string_view shaderType)
	{
		using enum resource::ShaderType;
		if (shaderType == "VERTEX") return VERTEX;
		if (shaderType == "FRAGMENT") return FRAGMENT;
		if (shaderType == "COMPUTE") return COMPUTE;
		if (shaderType == "GEOMETRY") return GEOMETRY;
		if (shaderType == "CONTROL") return TESS_CONTROL;
		if (shaderType == "EVALUATION") return TESS_EVALUATION;
		return UNKNOWN;
	}
}

