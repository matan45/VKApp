#include "Shader.hpp"
#include "Device.hpp"
#include "print/Logger.hpp"
#include <iostream>     // For input-output operations
#include <fstream>      // For file handling (ifstream)
#include <string>       // For using std::string to read lines

namespace core {
	Shader::Shader(Device& device) :device{ device }
	{

	}

	void Shader::readShader(std::string_view path, vk::ShaderStageFlagBits stage, std::string_view shaderName)
	{
		// Compile GLSL to SPIR-V
		std::vector<uint32_t> spirvCode = compileShaderToSPIRV(path, stage, shaderName);
		// Create Vulkan shader module
		createShaderModule(spirvCode);
	}

	vk::PipelineShaderStageCreateInfo Shader::createShaderStage() const
	{
		vk::PipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.stage = stageShader;
		shaderStageInfo.module = shaderModule.get();
		shaderStageInfo.pName = "main";  // Entry point function in the shader
		return shaderStageInfo;
	}

	void Shader::cleanUp()
	{
		shaderModule.reset();
	}

	std::vector<uint32_t> Shader::compileShaderToSPIRV(std::string_view path, vk::ShaderStageFlagBits stage, std::string_view shaderName)
	{
		std::string fileContent = readFile(path);
		shaderc_shader_kind kind;
		stageShader = stage;

		// Map Vulkan shader stage to shaderc shader kind
		switch (stage) {
			using enum vk::ShaderStageFlagBits;
		case eVertex: kind = shaderc_vertex_shader; break;
		case eFragment: kind = shaderc_fragment_shader; break;
		case eGeometry: kind = shaderc_geometry_shader; break;
		case eCompute: kind = shaderc_compute_shader; break;
		case eTessellationControl: kind = shaderc_tess_control_shader; break;
		case eTessellationEvaluation: kind = shaderc_tess_evaluation_shader; break;
		default:
			loggerError("Unsupported shader stage");
		}

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		// Compile GLSL to SPIR-V
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(fileContent, kind, shaderName.data(), options);

		// Check for compilation errors
		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::string errorMsg = result.GetErrorMessage();
			loggerError("Shader compilation failed for {}: {}", shaderName, errorMsg);
		}

		// Return the compiled SPIR-V code
		return { result.cbegin(), result.cend() };
	}

	void Shader::createShaderModule(const std::vector<uint32_t>& code)
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size() * sizeof(uint32_t);  // SPIR-V is a vector of 32-bit words
		createInfo.pCode = code.data();

		shaderModule = device.getLogicalDevice().createShaderModuleUnique(createInfo);
	}

	std::string Shader::readFile(std::string_view path) const
	{
		std::ifstream file(path.data());  // Create input file stream
		std::string file_content;
		if (file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();  // Read the entire file into a stringstream
			file_content = buffer.str();  // Convert the stringstream to a string


			file.close();  // Close the file when done
		}
		else {
			std::cout << "Failed to open the file." << std::endl;
		}
		return file_content;
	}

}