#include "IBL.hpp"
#include "../core/Texture.hpp"
#include <memory>

namespace render {

	IBL::IBL(core::Device& device, core::SwapChain& swapChain, std::vector<core::OffscreenResources>& offscreenResources) : device{ device },
		swapChain{ swapChain }, offscreenResources{ offscreenResources }
	{

	}

	void IBL::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
	}

	void IBL::init(std::string_view path)
	{
		auto hdrTexture = std::make_shared<core::Texture>(device);
		hdrTexture->loadHDRFromFile(path, vk::Format::eR16G16B16A16Sfloat, false);
	}

	void IBL::recreate()
	{
	}

	void IBL::cleanUp() const
	{
	}

	void IBL::generatePrefilteredCube()
	{
	}

	void IBL::generateIrradianceCube()
	{
	}

	void IBL::generateBRDFLUT()
	{
	}

}
