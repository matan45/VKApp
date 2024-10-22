#pragma once

namespace core {
	class Device;
	class SwapChain;
}
namespace imguiPass {
	class OffScreenViewPort;
}

namespace controllers {
	class OffScreenController
	{
	private:
		core::SwapChain& swapChain;
		core::Device& device;
		imguiPass::OffScreenViewPort* offScreen;
	public:
		explicit OffScreenController();
		~OffScreenController();

		void init();
		void cleanUp() const;

		void* render();
	};
}

