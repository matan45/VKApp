#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../core/OffScreen.hpp"

namespace core {
	class Device;
	class SwapChain;
	class Shader;
}

namespace render {
	class IBL
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;
		std::vector<core::OffscreenResources>& offscreenResources;

	public:
		explicit IBL(core::Device& device, core::SwapChain& swapChain, std::vector<core::OffscreenResources>& offscreenResources);
		~IBL() = default;

		void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) const;

		void init(std::string_view path);
		void recreate();
		void cleanUp() const;

	private:
		void generatePrefilteredCube();
		void generateIrradianceCube();
		void generateBRDFLUT();
	};
}


