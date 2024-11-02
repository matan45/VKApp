#include "IBL.hpp"
#include "../core/Utilities.hpp"
#include "print/Logger.hpp"
#include "../core/Device.hpp"
#include <memory>

namespace render
{
    IBL::IBL(core::Device& device, core::SwapChain& swapChain,
             std::vector<core::OffscreenResources>& offscreenResources) : device{device},
                                                                          swapChain{swapChain},
                                                                          offscreenResources{offscreenResources}
    {
        shaderIrradianceCube = std::make_shared<core::Shader>(device);
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
        poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().graphicsAndComputeFamily.value();
    }

    void IBL::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
    {
    }

    void IBL::init(std::string_view path)
    {
        hdrTexture = std::make_shared<core::Texture>(device);
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
        core::ImageInfoRequest cubeMapImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
        cubeMapImageRequest.format = vk::Format::eR16G16B16A16Sfloat;
        cubeMapImageRequest.layers = 6;
        cubeMapImageRequest.width = CUBE_MAP_SIZE;
        cubeMapImageRequest.height = CUBE_MAP_SIZE;
        core::Utilities::createImage(cubeMapImageRequest, imageIrradianceCube.cubeMapImage,
                                     imageIrradianceCube.cubeMapImageMemory);

        core::ImageViewInfoRequest cubeMapImageViewRequest(device.getLogicalDevice(), imageIrradianceCube.cubeMapImage);
        cubeMapImageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
        cubeMapImageViewRequest.layerCount = 6;
        cubeMapImageViewRequest.imageType = vk::ImageViewType::eCube;
        core::Utilities::createImageView(cubeMapImageViewRequest, imageIrradianceCube.cubeMapImageView);

        shaderIrradianceCube->readShader("../../resources/shaders/ibl/equirectangular_convolution.glsl");

        //SET UP RENDER PASS
        vk::AttachmentDescription colorAttachment;
        colorAttachment.format = vk::Format::eR16G16B16A16Sfloat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

        //DEFINE THE VERTEX BUFFER
        vk::VertexInputBindingDescription vertexInputBindingDescription;
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = sizeof(glm::vec3);
        vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

        std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
        vertexInputAttributes.push_back({0, 0, vk::Format::eR32G32Sfloat, 0});

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        {
            core::BufferInfoRequest bufferInfo(device.getLogicalDevice(), device.getPhysicalDevice());
            bufferInfo.size = sizeof(cubeVertices[0]) * cubeVertices.size();
            bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
            bufferInfo.properties = vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent;
            core::Utilities::createBuffer(bufferInfo, vertexBuffer, vertexBufferMemory);

            void* data;
            if (vk::Result result = device.getLogicalDevice().mapMemory(vertexBufferMemory, 0, bufferInfo.size, {},
                                                                        &data); result != vk::Result::eSuccess)
            {
                loggerError("failed to map memory");
            }
            memcpy(data, cubeVertices.data(), bufferInfo.size);
            device.getLogicalDevice().unmapMemory(vertexBufferMemory);
        }

        //DEFINE THE UNIFORM BUFFER LAYOUT
        vk::DescriptorPool descriptorPool;
        {
            std::vector<vk::DescriptorPoolSize> poolSizes(2);
            poolSizes[0].type = vk::DescriptorType::eCombinedImageSampler;
            poolSizes[0].descriptorCount = 1;
            poolSizes[1].type = vk::DescriptorType::eUniformBuffer;
            poolSizes[1].descriptorCount = 1;

            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 1;

            descriptorPool = device.getLogicalDevice().createDescriptorPool(poolInfo);
        }

        vk::DescriptorSetLayout descriptorSetLayout;
        {
            std::vector<vk::DescriptorSetLayoutBinding> bindings(2);

            bindings[0].binding = 0;
            bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex;
            bindings[0].pImmutableSamplers = nullptr;

            bindings[1].binding = 0;
            bindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
            bindings[1].descriptorCount = 1;
            bindings[1].stageFlags = vk::ShaderStageFlagBits::eFragment;
            bindings[1].pImmutableSamplers = nullptr;

            vk::DescriptorSetLayoutCreateInfo layoutInfo;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            descriptorSetLayout = device.getLogicalDevice().createDescriptorSetLayout(layoutInfo);
        }

        //DEFINE THE UNIFORM BUFFER AND SAMPLER
        vk::Buffer uniformBuffer;
        vk::DeviceMemory uniformBufferMemory;
        core::BufferInfoRequest bufferRequest(device.getLogicalDevice(), device.getPhysicalDevice());
        bufferRequest.usage = vk::BufferUsageFlagBits::eUniformBuffer;
        bufferRequest.size = sizeof(UniformBufferObject);
        core::Utilities::createBuffer(bufferRequest, uniformBuffer, uniformBufferMemory);

        vk::DescriptorSet descriptorSet;
        {
            vk::DescriptorSetAllocateInfo allocInfo;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptorSetLayout;

            descriptorSet = device.getLogicalDevice().allocateDescriptorSets(allocInfo)[0];
        }
        {
            vk::DescriptorImageInfo imageInfo;
            imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo.imageView = hdrTexture->getImageView();
            imageInfo.sampler = hdrTexture->getSampler();

            vk::WriteDescriptorSet descriptorWrite;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            device.getLogicalDevice().updateDescriptorSets(descriptorWrite, nullptr);
        }
        {
            vk::DescriptorBufferInfo bufferInfo;
            bufferInfo.buffer = uniformBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            vk::WriteDescriptorSet descriptorWrite;
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            device.getLogicalDevice().updateDescriptorSets(descriptorWrite, nullptr);
        }

        //DEFINE GRAPHICS PIPELINE
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        vk::Viewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(CUBE_MAP_SIZE);
        viewport.height = static_cast<float>(CUBE_MAP_SIZE);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor;
        scissor.offset = vk::Offset2D(0, 0);
        scissor.extent = vk::Extent2D(CUBE_MAP_SIZE, CUBE_MAP_SIZE);

        vk::PipelineViewportStateCreateInfo viewportState;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = vk::PolygonMode::eFill;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = vk::CullModeFlagBits::eBack;
        rasterizer.frontFace = vk::FrontFace::eClockwise;
        rasterizer.depthBiasEnable = VK_FALSE;

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
            | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = VK_FALSE;

        vk::PipelineColorBlendStateCreateInfo colorBlending;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        vk::PipelineLayout pipelineLayout = device.getLogicalDevice().createPipelineLayout(pipelineLayoutInfo);

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderIrradianceCube->getShaderStages().size());
        pipelineInfo.pStages = shaderIrradianceCube->getShaderStages().data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        vk::Pipeline graphicsPipeline = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;

        //Create frame buffers for each face.
        std::vector<vk::Framebuffer> frameBuffers(6);
        for (uint32_t face = 0; face < 6; ++face)
        {
            vk::ImageViewCreateInfo faceImageViewInfo{};
            faceImageViewInfo.image = imageIrradianceCube.cubeMapImage;
            faceImageViewInfo.viewType = vk::ImageViewType::e2D;
            faceImageViewInfo.format = vk::Format::eR16G16B16A16Sfloat;
            faceImageViewInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, face, 1};

            vk::ImageView faceImageView = device.getLogicalDevice().createImageView(faceImageViewInfo);

            vk::FramebufferCreateInfo framebufferInfo{};
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &faceImageView;
            framebufferInfo.width = CUBE_MAP_SIZE;
            framebufferInfo.height = CUBE_MAP_SIZE;
            framebufferInfo.layers = 1;

            frameBuffers[face] = device.getLogicalDevice().createFramebuffer(framebufferInfo);
        }

        //setUp command buffer
        vk::UniqueCommandPool commandPool = device.getLogicalDevice().createCommandPoolUnique(poolInfo);

        vk::UniqueCommandBuffer commandBuffer = core::Utilities::beginSingleTimeCommands(
            device.getLogicalDevice(), commandPool.get());

        //DRAW COMMAND 
        for (uint32_t face = 0; face < 6; ++face)
        {
            updateUniformBuffer(CameraViewMatrix::captureViews[face], CameraViewMatrix::captureProjection, uniformBufferMemory);

            vk::ClearValue clearColor{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
            vk::RenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.renderPass = renderPass;
            renderPassBeginInfo.framebuffer = frameBuffers[face];
            renderPassBeginInfo.renderArea = vk::Rect2D({0, 0}, {CUBE_MAP_SIZE, CUBE_MAP_SIZE});
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearColor;

            commandBuffer.get().beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
            commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
            commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout,
                                                   0, descriptorSet, {});

            // Bind vertex buffer and draw
            vk::DeviceSize offsets[] = {0};
            commandBuffer.get().bindVertexBuffers(0, vertexBuffer, offsets);
            commandBuffer.get().draw(static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);

            commandBuffer.get().endRenderPass();
        }

        // Create a fence to wait for completion
        vk::Fence renderFence = device.getLogicalDevice().createFence({});
        core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandBuffer, renderFence);

        // Wait for the command buffer to finish executing
        vk::Result result = device.getLogicalDevice().waitForFences(renderFence, VK_TRUE, UINT64_MAX);
        if (result != vk::Result::eSuccess)
        {
            loggerError("Failed to to wait for Fence IBL:");
        }

        vk::UniqueCommandBuffer transitionCommandBuffer = core::Utilities::beginSingleTimeCommands(
            device.getLogicalDevice(), commandPool.get());
        core::Utilities::transitionImageLayout(
            transitionCommandBuffer.get(),
            imageIrradianceCube.cubeMapImage,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageAspectFlagBits::eColor
        );
        core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandBuffer);

        //TODO cleanUp
        device.getLogicalDevice().destroyFence(renderFence);
        for (auto framebuffer : frameBuffers)
        {
            device.getLogicalDevice().destroyFramebuffer(framebuffer);
        }
        //TODO also if it works we can optimise it some buffers like the vertex buffer need to be define only ones
    }

    void IBL::generateBRDFLUT()
    {
    }

    void IBL::updateUniformBuffer(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                  const vk::DeviceMemory& uniformBufferMemory)
    {
        UniformBufferObject ubo;
        ubo.view = viewMatrix;
        ubo.projection = projectionMatrix;

        void* data;
        vk::Result result = device.getLogicalDevice().mapMemory(uniformBufferMemory, 0, sizeof(ubo), {}, &data);
        if (result == vk::Result::eSuccess)
        {
            memcpy(data, &ubo, sizeof(ubo));
            device.getLogicalDevice().unmapMemory(uniformBufferMemory);
        }
    }
}
