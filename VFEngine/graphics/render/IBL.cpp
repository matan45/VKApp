#include "IBL.hpp"
#include "../core/Texture.hpp"
#include "../core/Utilities.hpp"
#include <memory>

#include "../core/Device.hpp"

namespace render
{
    IBL::IBL(core::Device& device, core::SwapChain& swapChain,
             std::vector<core::OffscreenResources>& offscreenResources) : device{device},
                                                                          swapChain{swapChain},
                                                                          offscreenResources{offscreenResources}
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
        uint32_t cubemapSize = 512;
        vk::DeviceMemory cubeMapImageMemory;
        vk::Image cubeMapImage;
        core::ImageInfoRequest cubeMapImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
        cubeMapImageRequest.format = vk::Format::eR16G16B16A16Sfloat;
        cubeMapImageRequest.layers = 6;
        cubeMapImageRequest.width = cubemapSize;
        cubeMapImageRequest.height = cubemapSize;
        core::Utilities::createImage(cubeMapImageRequest, cubeMapImage, cubeMapImageMemory);

        vk::ImageView cubeMapImageView;
        core::ImageViewInfoRequest cubeMapImageViewRequest(device.getLogicalDevice(), cubeMapImage);
        cubeMapImageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
        cubeMapImageViewRequest.layerCount = 6;
        cubeMapImageViewRequest.imageType = vk::ImageViewType::eCube;
        core::Utilities::createImageView(cubeMapImageViewRequest, cubeMapImageView);

        //missing shader and render pass and pipeline
        // Create frame buffers for each face.
        /*std::vector<vk::Framebuffer> frameBuffers(6);
        for (uint32_t i = 0; i < 6; ++i) {
            vk::ImageViewCreateInfo faceViewInfo;
            faceViewInfo.viewType = vk::ImageViewType::e2D;
            faceViewInfo.subresourceRange.baseArrayLayer = i;
            faceViewInfo.subresourceRange.layerCount = 1;

            vk::ImageView faceImageView = device.getLogicalDevice().createImageView(faceViewInfo);

            vk::FramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &faceImageView;
            framebufferInfo.width = cubemapSize;
            framebufferInfo.height = cubemapSize;
            framebufferInfo.layers = 1;
            frameBuffers[i] = device.getLogicalDevice().createFramebuffer(framebufferInfo);
        }*/
    }

    void IBL::generateBRDFLUT()
    {
    }
}
