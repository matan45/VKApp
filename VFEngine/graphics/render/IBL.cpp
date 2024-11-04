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
        brdfLUTShader = std::make_shared<core::Shader>(device);
        prefilterShader = std::make_shared<core::Shader>(device);
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
        poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().graphicsAndComputeFamily.value();
    }

    void IBL::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
    {
    }

    void IBL::init(std::string_view path)
	{

        hdrTexture = std::make_shared<core::Texture>(device);
        hdrTexture->loadHDRFromFile(path, vk::Format::eR8G8B8A8Srgb, false);
        generateIrradianceCube();
        generateBRDFLUT();
        //generatePrefilteredCube();
    }

    void IBL::recreate()
    {
    }

    void IBL::cleanUp() const
    {
    }

    void IBL::generatePrefilteredCube()
    {
        const uint32_t mipLevels = static_cast<uint32_t>(std::log2(CUBE_MAP_SIZE)) + 1;

        // 1. Create the cubemap image with mip levels
        core::ImageInfoRequest prefilterImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
        prefilterImageRequest.format = vk::Format::eR16G16B16A16Sfloat;
        prefilterImageRequest.width = CUBE_MAP_SIZE;
        prefilterImageRequest.height = CUBE_MAP_SIZE;
        prefilterImageRequest.layers = 6;
        prefilterImageRequest.mipLevels = mipLevels;
        prefilterImageRequest.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        core::Utilities::createImage(prefilterImageRequest, prefilterImage.image, prefilterImage.imageMemory);

        core::ImageViewInfoRequest imageViewRequest(device.getLogicalDevice(), prefilterImage.image);
        imageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
        imageViewRequest.layerCount = 6;
        imageViewRequest.imageType = vk::ImageViewType::eCube;
        imageViewRequest.mipLevels = mipLevels;
        core::Utilities::createImageView(imageViewRequest, prefilterImage.imageView);

        prefilterShader->readShader("../../resources/shaders/ibl/prefilter.glsl");

        // 2. Create a render pass for the cubemap with multiple mip levels
        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format = vk::Format::eR16G16B16A16Sfloat;
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::AttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);
    }

    void IBL::generateIrradianceCube()
    {
        core::ImageInfoRequest cubeMapImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
        cubeMapImageRequest.format = vk::Format::eR16G16B16A16Sfloat;
        cubeMapImageRequest.layers = 6;
        cubeMapImageRequest.width = CUBE_MAP_SIZE;
        cubeMapImageRequest.height = CUBE_MAP_SIZE;
        cubeMapImageRequest.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment;
        cubeMapImageRequest.imageFlags = vk::ImageCreateFlagBits::eCubeCompatible;
        core::Utilities::createImage(cubeMapImageRequest, imageIrradianceCube.image,
                                     imageIrradianceCube.imageMemory);

        core::ImageViewInfoRequest cubeMapImageViewRequest(device.getLogicalDevice(), imageIrradianceCube.image);
        cubeMapImageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
        cubeMapImageViewRequest.layerCount = 6;
        cubeMapImageViewRequest.imageType = vk::ImageViewType::eCube;
        core::Utilities::createImageView(cubeMapImageViewRequest, imageIrradianceCube.imageView);

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

        vk::SubpassDescription subPass;
        subPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subPass.colorAttachmentCount = 1;
        subPass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency dependency;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eTransfer;           // Use eTransfer for transfer write
		dependency.srcAccessMask = vk::AccessFlagBits::eTransferWrite;            // Transfer write access
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subPass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

        //DEFINE THE VERTEX BUFFER
        vk::VertexInputBindingDescription vertexInputBindingDescription;
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = sizeof(glm::vec3);
        vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

        std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
        vertexInputAttributes.push_back({0, 0, vk::Format::eR32G32B32Sfloat, 0});

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

            bindings[1].binding = 1;
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
            descriptorWrite.dstBinding = 1;
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
        pipelineLayoutInfo.setLayoutCount = 1;
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
            faceImageViewInfo.image = imageIrradianceCube.image;
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
            updateUniformBuffer(CameraViewMatrix::captureViews[face], CameraViewMatrix::captureProjection,
                                uniformBufferMemory);

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
            imageIrradianceCube.image,
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
        // BRDF LUT image setup
        core::ImageInfoRequest brdfLUTImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
        brdfLUTImageRequest.format = vk::Format::eR16G16Sfloat;
        brdfLUTImageRequest.width = CUBE_MAP_SIZE;
        brdfLUTImageRequest.height = CUBE_MAP_SIZE;
        brdfLUTImageRequest.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        core::Utilities::createImage(brdfLUTImageRequest, brdfLUTImage.image, brdfLUTImage.imageMemory);

        // BRDF LUT image view
        core::ImageViewInfoRequest imageViewRequest(device.getLogicalDevice(), brdfLUTImage.image);
        imageViewRequest.format = vk::Format::eR16G16Sfloat;
        core::Utilities::createImageView(imageViewRequest, brdfLUTImage.imageView);

        brdfLUTShader->readShader("../../resources/shaders/ibl/brdf.glsl");

        //DEFINE THE VERTEX BUFFER
        vk::VertexInputBindingDescription vertexInputBindingDescription;
        vertexInputBindingDescription.binding = 0;
        vertexInputBindingDescription.stride = sizeof(QuadVertex);
        vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

        std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
        vertexInputAttributes.push_back({0, 0, vk::Format::eR32G32B32Sfloat, offsetof(QuadVertex, position)});
        vertexInputAttributes.push_back({1, 0, vk::Format::eR32G32Sfloat, offsetof(QuadVertex, texture)});

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        {
            core::BufferInfoRequest bufferInfo(device.getLogicalDevice(), device.getPhysicalDevice());
            bufferInfo.size = sizeof(quad[0]) * quad.size();
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
            memcpy(data, quad.data(), bufferInfo.size);
            device.getLogicalDevice().unmapMemory(vertexBufferMemory);
        }

        //SET UP RENDER PASS
        vk::AttachmentDescription colorAttachment;
        colorAttachment.format = vk::Format::eR32G32Sfloat;  // 2 channels, 32-bit floats
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

        vk::SubpassDescription subPass;
        subPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subPass.colorAttachmentCount = 1;
        subPass.pColorAttachments = &colorAttachmentRef;

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
        renderPassInfo.pSubpasses = &subPass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

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
        vk::PipelineLayout pipelineLayout = device.getLogicalDevice().createPipelineLayout(pipelineLayoutInfo);

        vk::GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo.stageCount = static_cast<uint32_t>(brdfLUTShader->getShaderStages().size());
        pipelineInfo.pStages = brdfLUTShader->getShaderStages().data();
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

        // Framebuffer for BRDF LUT rendering
        vk::FramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &brdfLUTImage.imageView;
        framebufferInfo.width = CUBE_MAP_SIZE;
        framebufferInfo.height = CUBE_MAP_SIZE;
        framebufferInfo.layers = 1;

        vk::Framebuffer framebuffer = device.getLogicalDevice().createFramebuffer(framebufferInfo);

        // Command buffer recording
        vk::UniqueCommandPool commandPool = device.getLogicalDevice().createCommandPoolUnique(poolInfo);
        vk::UniqueCommandBuffer commandBuffer = core::Utilities::beginSingleTimeCommands(
            device.getLogicalDevice(), commandPool.get());

        vk::ClearValue clearColor{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}};
        vk::RenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffer;
        renderPassBeginInfo.renderArea = vk::Rect2D({0, 0}, {CUBE_MAP_SIZE, CUBE_MAP_SIZE});
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        vk::DeviceSize offsets[] = {0};
        commandBuffer->bindVertexBuffers(0, vertexBuffer, offsets);
        commandBuffer->draw(6, 1, 0, 0);
        commandBuffer->endRenderPass();

        // Execute and wait
        vk::Fence renderFence = device.getLogicalDevice().createFence({});
        core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandBuffer, renderFence);
        vk::Result result = device.getLogicalDevice().waitForFences(renderFence, VK_TRUE, UINT64_MAX);
        if (result != vk::Result::eSuccess)
        {
            loggerError("Failed to to wait for Fence BRDFLUT:");
        }

        // Transition image layout to shader-readable
        vk::UniqueCommandBuffer transitionCommandBuffer = core::Utilities::beginSingleTimeCommands(
            device.getLogicalDevice(), commandPool.get());
        core::Utilities::transitionImageLayout(
            transitionCommandBuffer.get(),
            brdfLUTImage.image,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal,
            vk::ImageAspectFlagBits::eColor
        );
        core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), transitionCommandBuffer);
        
        //TODO cleanUp
        device.getLogicalDevice().destroyFence(renderFence);
        device.getLogicalDevice().destroyFramebuffer(framebuffer);
        //TODO also if it works we can optimise it some buffers like the vertex buffer need to be define only ones
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

    //TODO use sub pass render
    /*
    void IBL::generateIBLTextures()
{
    // Attachment descriptions
    vk::AttachmentDescription irradianceAttachment{};
    irradianceAttachment.format = vk::Format::eR16G16B16A16Sfloat;
    irradianceAttachment.samples = vk::SampleCountFlagBits::e1;
    irradianceAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    irradianceAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    irradianceAttachment.initialLayout = vk::ImageLayout::eUndefined;
    irradianceAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentDescription brdfLUTAttachment{};
    brdfLUTAttachment.format = vk::Format::eR16G16B16A16Sfloat;
    brdfLUTAttachment.samples = vk::SampleCountFlagBits::e1;
    brdfLUTAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    brdfLUTAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    brdfLUTAttachment.initialLayout = vk::ImageLayout::eUndefined;
    brdfLUTAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentDescription prefilteredAttachment{};
    prefilteredAttachment.format = vk::Format::eR16G16B16A16Sfloat;
    prefilteredAttachment.samples = vk::SampleCountFlagBits::e1;
    prefilteredAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    prefilteredAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    prefilteredAttachment.initialLayout = vk::ImageLayout::eUndefined;
    prefilteredAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Attachment references for subpasses
    //Attachment Index (vk::AttachmentReference): This index (0, 1, 2 in our case) refers to the attachment description in the attachments array in the render pass. 
    vk::AttachmentReference irradianceAttachmentRef{0, vk::ImageLayout::eColorAttachmentOptimal};
    vk::AttachmentReference brdfLUTAttachmentRef{1, vk::ImageLayout::eColorAttachmentOptimal};
    vk::AttachmentReference prefilteredAttachmentRef{2, vk::ImageLayout::eColorAttachmentOptimal};

    // Subpass 1: Irradiance Cube
    vk::SubpassDescription subpass1{};
    subpass1.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass1.colorAttachmentCount = 1;
    subpass1.pColorAttachments = &irradianceAttachmentRef;

    // Subpass 2: BRDF LUT
    vk::SubpassDescription subpass2{};
    subpass2.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass2.colorAttachmentCount = 1;
    subpass2.pColorAttachments = &brdfLUTAttachmentRef;

    // Subpass 3: Prefiltered Environment Map
    vk::SubpassDescription subpass3{};
    subpass3.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass3.colorAttachmentCount = 1;
    subpass3.pColorAttachments = &prefilteredAttachmentRef;

    // Dependencies between subpasses
    std::array<vk::SubpassDependency, 3> dependencies{};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[0].srcAccessMask = vk::AccessFlagBits::eNoneKHR;
    dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    dependencies[2].srcSubpass = 1;
    dependencies[2].dstSubpass = 2;
    dependencies[2].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[2].srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[2].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[2].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    // Render pass setup
    std::array<vk::AttachmentDescription, 3> attachments = {irradianceAttachment, brdfLUTAttachment, prefilteredAttachment};
    std::array<vk::SubpassDescription, 3> subpasses = {subpass1, subpass2, subpass3};

    vk::RenderPassCreateInfo renderPassInfo{};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
    renderPassInfo.pSubpasses = subpasses.data();
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);
    // Ensure this render pass is used correctly in the pipeline creation for each subpass.

    // Continue with creating framebuffers, command buffers, and pipelines for each subpass...
}


    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

// Subpass 1: Irradiance Cube
commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, irradiancePipeline);
commandBuffer.draw(...);
commandBuffer.nextSubpass(vk::SubpassContents::eInline);

// Subpass 2: BRDF LUT
commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, brdfPipeline);
commandBuffer.draw(...);
commandBuffer.nextSubpass(vk::SubpassContents::eInline);

// Subpass 3: Prefiltered Environment Map
commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, prefilteredPipeline);
commandBuffer.draw(...);

commandBuffer.endRenderPass();

     **/
}
