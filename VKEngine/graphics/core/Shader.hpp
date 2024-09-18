#pragma once

#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>
#include <string>

namespace core {
	class Device;

	class Shader
	{
	private:
		Device& device;
		vk::UniqueShaderModule shaderModule;
		vk::ShaderStageFlagBits stageShader;
	public:
		explicit Shader(Device& device);
		~Shader() = default;
		//todo move it to the resource class
		void readShader(const std::string& sourceCode, vk::ShaderStageFlagBits stage, const std::string& shaderName);
		vk::PipelineShaderStageCreateInfo createShaderStage() const;
		const vk::ShaderModule& getShaderModule() const { return shaderModule.get(); }
	private:
		std::vector<uint32_t> compileShaderToSPIRV(const std::string& sourceCode, vk::ShaderStageFlagBits stage, const std::string& shaderName);
		void createShaderModule(const std::vector<uint32_t>& code);
	};
}


