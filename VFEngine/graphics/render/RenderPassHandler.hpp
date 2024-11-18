#pragma once
#include "../core/OffScreen.hpp"

namespace core
{
    class Device;
    class SwapChain;
}


namespace render
{
    class ClearColor;
    class IBL;

    class RenderPassHandler
    {
    private:
        core::Device& device;
        core::SwapChain& swapChain;

        ClearColor* clearColor{nullptr};
        IBL* iblRenderer{nullptr};

        core::OffscreenResources& offscreenResources;

    public:
        explicit RenderPassHandler(core::Device& device, core::SwapChain& swapChain,
                                   core::OffscreenResources& offscreenResources);
        ~RenderPassHandler();

        void init();

        void recreate() const;

        IBL* getIBL() const { return iblRenderer; }

        void cleanUp() const;

        void draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;
    };
}
