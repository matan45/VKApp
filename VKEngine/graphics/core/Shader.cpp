#include "Shader.hpp"
#include "Device.hpp"
#include "log/Logger.hpp"

namespace core {
	Shader::Shader(Device& device) :device{ device }
	{

	}

	void Shader::readShader(const std::string& sourceCode, vk::ShaderStageFlagBits stage, const std::string& shaderName)
	{
		// Compile GLSL to SPIR-V
		std::vector<uint32_t> spirvCode = compileShaderToSPIRV(sourceCode, stage, shaderName);
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

	std::vector<uint32_t> Shader::compileShaderToSPIRV(const std::string& sourceCode, vk::ShaderStageFlagBits stage, const std::string& shaderName)
	{
		shaderc_shader_kind kind;
		stageShader = stage;

		// Map Vulkan shader stage to shaderc shader kind
		switch (stage) {
			using enum vk::ShaderStageFlagBits;
		case eVertex:
			kind = shaderc_vertex_shader;
			break;
		case eFragment:
			kind = shaderc_fragment_shader;
			break;
		case eGeometry:
			kind = shaderc_geometry_shader;
			break;
		case eCompute:
			kind = shaderc_compute_shader;
			break;
		case eTessellationControl:
			kind = shaderc_tess_control_shader;
			break;
		case eTessellationEvaluation:
			kind = shaderc_tess_evaluation_shader;
			break;
		default:
			loggerError("Unsupported shader stage");
		}

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Enable optimizations (optional)
		options.SetOptimizationLevel(shaderc_optimization_level_performance);

		// Compile GLSL to SPIR-V
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(sourceCode, kind, shaderName.c_str(), options);

		// Check for compilation errors
		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			loggerError("Shader compilation failed: {}", std::string(result.GetErrorMessage()));
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

}
