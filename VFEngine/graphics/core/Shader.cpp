#include "Shader.hpp"
#include "Device.hpp"
#include "print/Logger.hpp"  
#include <filesystem>

namespace core {
	Shader::Shader(Device& device) :device{ device }
	{

	}

	void Shader::readShader(std::string_view path)
	{
		auto futureShaders = resource::ResourceManager::loadShaderAsync(path);
		// Extract the shader name from the file path
		std::string shaderName = std::filesystem::path(path).stem().string();

		auto shaders = futureShaders.get();
		if (!shaders) {
			loggerError("Failed to load shaders from file: {}", path);
			return;
		}

		for (const auto& shader : *shaders) {
			vk::ShaderStageFlagBits stage = shaderTypeToVulkanStage(shader.type);
			// Compile GLSL to SPIR-V
			std::vector<uint32_t> spirvCode = compileShaderToSPIRV(shader.source, stage, shaderName);
			// Create Vulkan shader module
			createShaderModule(spirvCode, stage);
		}
		
	}

	void Shader::cleanUp()
	{
		shaderModules.clear();
	}

	std::vector<uint32_t> Shader::compileShaderToSPIRV(std::string_view source, vk::ShaderStageFlagBits stage, std::string_view shaderName) const
	{
		shaderc_shader_kind kind;

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
		shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.data(), kind, shaderName.data(), options);

		// Check for compilation errors
		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::string errorMsg = result.GetErrorMessage();
			loggerError("Shader compilation failed for {}: {}", shaderName, errorMsg);
		}

		// Return the compiled SPIR-V code
		return { result.cbegin(), result.cend() };
	}

	void Shader::createShaderModule(const std::vector<uint32_t>& code, vk::ShaderStageFlagBits stage)
	{
		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size() * sizeof(uint32_t);
		createInfo.pCode = code.data();

		vk::UniqueShaderModule shaderModule = device.getLogicalDevice().createShaderModuleUnique(createInfo);

		vk::PipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.stage = stage;
		shaderStageInfo.module = shaderModule.get();
		shaderStageInfo.pName = "main";  // Entry point function in the shader

		// Store the created shader resource and stage info
		shaderModules.push_back(std::move(shaderModule));
		shaderStages.push_back(shaderStageInfo);
	}

	vk::ShaderStageFlagBits Shader::shaderTypeToVulkanStage(resource::ShaderType shaderType) const
	{
		switch (shaderType) {
		using enum vk::ShaderStageFlagBits;
		using enum resource::ShaderType;
		case VERTEX:
			return eVertex;
		case FRAGMENT:
			return eFragment;
		case COMPUTE:
			return eCompute;
		case GEOMETRY:
			return eGeometry;
		case TESS_CONTROL:
			return eTessellationControl;
		case TESS_EVALUATION:
			return eTessellationEvaluation;
		default:
			loggerError("Unsupported ShaderType.");
			return vk::ShaderStageFlagBits();
		}
	}

}