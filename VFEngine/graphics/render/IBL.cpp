#include "IBL.hpp"
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
        shader = std::make_shared<core::Shader>(device);
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
        shader->readShader("../../resources/shaders/ibl/equirectangular_convolution.glsl");

        vk::AttachmentDescription colorAttachment;
        colorAttachment.format = vk::Format::eR16G16B16A16Sfloat; // Matches the cubemap image format.
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear; // Clear the attachment at the start.
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore; // Store the result.
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined; // Starting layout.
        colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal; // Ending layout.

        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass;
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::RenderPassCreateInfo renderPassInfo;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        // Define subpass dependencies to handle layout transitions.
        vk::SubpassDependency dependency;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

        vk::DescriptorSetLayout descriptorSetLayout;
        {
            std::vector<vk::DescriptorSetLayoutBinding> bindings(1);

            // Binding 0: Environment cubemap (input image sampler)
            bindings[0].binding = 0;
            bindings[0].descriptorType = vk::DescriptorType::eCombinedImageSampler;
            bindings[0].descriptorCount = 1;
            bindings[0].stageFlags = vk::ShaderStageFlagBits::eFragment; // Assuming sampling in fragment shader
            bindings[0].pImmutableSamplers = nullptr;

            vk::DescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();

            descriptorSetLayout = device.getLogicalDevice().createDescriptorSetLayout(layoutInfo);
        }

        vk::DescriptorPool descriptorPool;
        {
            std::vector<vk::DescriptorPoolSize> poolSizes(1);
            poolSizes[0].type = vk::DescriptorType::eCombinedImageSampler;
            poolSizes[0].descriptorCount = 1;

            vk::DescriptorPoolCreateInfo poolInfo{};
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 1;

            descriptorPool = device.getLogicalDevice().createDescriptorPool(poolInfo);
        }

        // Allocate the descriptor set
        vk::DescriptorSet descriptorSet;
        {
            vk::DescriptorSetAllocateInfo allocInfo{};
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &descriptorSetLayout;

            descriptorSet = device.getLogicalDevice().allocateDescriptorSets(allocInfo)[0];
        }
        {
            vk::DescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imageInfo.imageView = hdrTexture->getImageView();
            imageInfo.sampler = hdrTexture->getSampler();

            vk::WriteDescriptorSet descriptorWrite{};
            descriptorWrite.dstSet = descriptorSet;
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            device.getLogicalDevice().updateDescriptorSets(descriptorWrite, nullptr);
        }

        vk::VertexInputBindingDescription vertexInputBindingDescription;
        std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

        // Input assembly.
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.topology = vk::PrimitiveTopology::eTriangleList; // Draw triangles.
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /*
               
        
                // Input assembly.
                VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
                inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Draw triangles.
                inputAssembly.primitiveRestartEnable = VK_FALSE;
        
                // Viewport and scissor for rendering to the cubemap face.
                VkViewport viewport = {};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(cubemapSize);
                viewport.height = static_cast<float>(cubemapSize);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
        
                VkRect2D scissor = {};
                scissor.offset = {0, 0};
                scissor.extent = {cubemapSize, cubemapSize};
        
                VkPipelineViewportStateCreateInfo viewportState = {};
                viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewportState.viewportCount = 1;
                viewportState.pViewports = &viewport;
                viewportState.scissorCount = 1;
                viewportState.pScissors = &scissor;
        
                // Rasterizer configuration.
                VkPipelineRasterizationStateCreateInfo rasterizer = {};
                rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable = VK_FALSE;
                rasterizer.rasterizerDiscardEnable = VK_FALSE;
                rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                rasterizer.lineWidth = 1.0f;
                rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
                rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
                rasterizer.depthBiasEnable = VK_FALSE;
        
                // Multisampling - disabled for this task.
                VkPipelineMultisampleStateCreateInfo multisampling = {};
                multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable = VK_FALSE;
                multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        
                // Color blending - simple blending.
                VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
                colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = VK_FALSE;
        
                VkPipelineColorBlendStateCreateInfo colorBlending = {};
                colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.logicOpEnable = VK_FALSE;
                colorBlending.attachmentCount = 1;
                colorBlending.pAttachments = &colorBlendAttachment;
        
                // Shader stages.
                VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
                vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module = vertShaderModule; // Your compiled vertex shader module.
                vertShaderStageInfo.pName = "main";
        
                VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
                fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module = fragShaderModule; // Your compiled fragment shader module.
                fragShaderStageInfo.pName = "main";
        
                VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
        
                // Pipeline layout (descriptor sets and push constants).
                VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
                pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 1;
                pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
                pipelineLayoutInfo.pushConstantRangeCount = 1;
                VkPushConstantRange pushConstantRange = {};
                pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                pushConstantRange.offset = 0;
                pushConstantRange.size = sizeof(PushConstantData);
                pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        
                VkPipelineLayout pipelineLayout;
                if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create pipeline layout!");
                }
        
                // Create the graphics pipeline.
                VkGraphicsPipelineCreateInfo pipelineInfo = {};
                pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.stageCount = 2;
                pipelineInfo.pStages = shaderStages;
                pipelineInfo.pVertexInputState = &vertexInputInfo;
                pipelineInfo.pInputAssemblyState = &inputAssembly;
                pipelineInfo.pViewportState = &viewportState;
                pipelineInfo.pRasterizationState = &rasterizer;
                pipelineInfo.pMultisampleState = &multisampling;
                pipelineInfo.pColorBlendState = &colorBlending;
                pipelineInfo.layout = pipelineLayout;
                pipelineInfo.renderPass = renderPass;
                pipelineInfo.subpass = 0;
                pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        
                VkPipeline pipeline;
                if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
                {
                    throw std::runtime_error("failed to create graphics pipeline!");
                }
        */


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


        std::array<glm::mat4, 6> captureViews = {
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
        };
        /*
                for (uint32_t i = 0; i < 6; ++i) {
                    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(commandPool);
        
                    VkRenderPassBeginInfo renderPassInfo = {};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderPassInfo.renderPass = renderPass; // Render pass for cubemap rendering.
                    renderPassInfo.framebuffer = framebuffers[i]; // Framebuffer for the current face.
                    renderPassInfo.renderArea.offset = {0, 0};
                    renderPassInfo.renderArea.extent = {cubemapSize, cubemapSize};
                    renderPassInfo.clearValueCount = 1;
                    VkClearValue clearValue = {1.0f, 1.0f, 1.0f, 1.0f}; // Clear color.
                    renderPassInfo.pClearValues = &clearValue;
        
                    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        
                    // Bind the graphics pipeline and descriptor set for the environment map.
                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
        
                    // Push view and projection matrices for each cubemap face using push constants.
                    PushConstantData pushConstant = {};
                    pushConstant.view = captureViews[i];
                    pushConstant.proj = captureProjection;
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantData), &pushConstant);
        
                    // Draw the cube. Assumes the vertex buffer for a unit cube is already bound.
                    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
        
                    vkCmdEndRenderPass(commandBuffer);
        
                    EndSingleTimeCommands(commandBuffer); // Submit and execute the recorded commands.
                }
        */
    }

    void IBL::generateBRDFLUT()
    {
    }
}
