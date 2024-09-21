#pragma once

#include <vulkan/vulkan.hpp>
#include <shaderc/shaderc.hpp>
#include <string>
#include <string_view>

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
		void readShader(std::string_view path, vk::ShaderStageFlagBits stage, std::string_view shaderName);
		vk::PipelineShaderStageCreateInfo createShaderStage() const;
		const vk::ShaderModule& getShaderModule() const { return shaderModule.get(); }
		void cleanUp();
	private:
		std::vector<uint32_t> compileShaderToSPIRV(std::string_view path, vk::ShaderStageFlagBits stage, std::string_view shaderName);
		void createShaderModule(const std::vector<uint32_t>& code);
		//todo move it to the resource class
		std::string readFile(std::string_view path) const;
	};
}


