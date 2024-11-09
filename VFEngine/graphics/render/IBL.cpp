#include "IBL.hpp"
#include "../core/Utilities.hpp"
#include "print/Logger.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include <memory>


namespace render
{
	IBL::IBL(core::Device& device, core::SwapChain& swapChain,
		std::vector<core::OffscreenResources>& offscreenResources) : device{ device },
		swapChain{ swapChain },
		offscreenResources{ offscreenResources }
	{
		shaderIrradianceCube = std::make_shared<core::Shader>(device);
		brdfLUTShader = std::make_shared<core::Shader>(device);
		prefilterShader = std::make_shared<core::Shader>(device);
		skyboxShader = std::make_shared<core::Shader>(device);
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
		poolInfo.queueFamilyIndex = device.getQueueFamilyIndices().graphicsAndComputeFamily.value();
	}

	void IBL::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		updateUniformBuffer(CameraViewMatrix::captureViews[1], CameraViewMatrix::captureProjection, uniformBufferMemory);
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[imageIndex];
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = swapChain.getSwapchainExtent();

		std::array<vk::ClearValue, 1> clearValues{};
		clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Begin render pass
		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		// Bind the graphics pipeline
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet,
			{});

		// Bind vertex buffer
		vk::DeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, vertexBuffer, offsets);
		commandBuffer.draw(static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);

		// End render pass
		commandBuffer.endRenderPass();
	}

	void IBL::init(std::string_view path)
	{
		hdrTexture = std::make_shared<core::Texture>(device);
		hdrTexture->loadHDRFromFile(path, vk::Format::eR32G32B32Sfloat, false);

		generateIrradianceCube();
		generateBRDFLUT();
		generatePrefilteredCube();
		drawCube();
	}

	void IBL::recreate()
	{
	}

	void IBL::cleanUp() const
	{
	}

	void IBL::generatePrefilteredCube()
	{
		const uint32_t mipLevels = static_cast<uint32_t>(floor(log2(CUBE_MAP_SIZE))) + 1;

		core::ImageInfoRequest cubeMapImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
		cubeMapImageRequest.format = vk::Format::eR16G16B16A16Sfloat;
		cubeMapImageRequest.layers = 6;
		cubeMapImageRequest.mipLevels = mipLevels;
		cubeMapImageRequest.width = CUBE_MAP_SIZE;
		cubeMapImageRequest.height = CUBE_MAP_SIZE;
		cubeMapImageRequest.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eColorAttachment;
		cubeMapImageRequest.imageFlags = vk::ImageCreateFlagBits::eCubeCompatible;
		core::Utilities::createImage(cubeMapImageRequest, prefilterImage.image,
			prefilterImage.imageMemory);

		core::ImageViewInfoRequest cubeMapImageViewRequest(device.getLogicalDevice(), prefilterImage.image);
		cubeMapImageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
		cubeMapImageViewRequest.layerCount = 6;
		cubeMapImageViewRequest.mipLevels = mipLevels;
		cubeMapImageViewRequest.imageType = vk::ImageViewType::eCube;
		core::Utilities::createImageView(cubeMapImageViewRequest, prefilterImage.imageView);


		prefilterShader->readShader("../../resources/shaders/ibl/prefilter.glsl");

		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		prefilterImage.sampler = device.getLogicalDevice().createSampler(samplerInfo);

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

		// Use subpass dependencies for layout transitions
		std::array<vk::SubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
		dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
			vk::AccessFlagBits::eColorAttachmentWrite;
		dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
			vk::AccessFlagBits::eColorAttachmentWrite;
		dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
		dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		vk::RenderPassCreateInfo renderPassInfo{};
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

		//DEFINE THE VERTEX BUFFER
		vk::VertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(glm::vec3);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
		vertexInputAttributes.push_back({ 0, 0, vk::Format::eR32G32B32Sfloat, 0 });

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
			poolSizes[1].descriptorCount = 2;

			vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 1;

			descriptorPool = device.getLogicalDevice().createDescriptorPool(poolInfo);
		}

		vk::DescriptorSetLayout descriptorSetLayout;
		{
			std::vector<vk::DescriptorSetLayoutBinding> bindings(3);

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

			bindings[2].binding = 2;
			bindings[2].descriptorType = vk::DescriptorType::eUniformBuffer;
			bindings[2].descriptorCount = 1;
			bindings[2].stageFlags = vk::ShaderStageFlagBits::eFragment;
			bindings[2].pImmutableSamplers = nullptr;

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
		bufferRequest.properties = vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent;
		bufferRequest.size = sizeof(UniformBufferObject);
		core::Utilities::createBuffer(bufferRequest, uniformBuffer, uniformBufferMemory);

		vk::Buffer runiformBuffer;
		vk::DeviceMemory runiformBufferMemory;
		core::BufferInfoRequest rbufferRequest(device.getLogicalDevice(), device.getPhysicalDevice());
		rbufferRequest.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		rbufferRequest.properties = vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent;
		rbufferRequest.size = sizeof(float);
		core::Utilities::createBuffer(rbufferRequest, runiformBuffer, runiformBufferMemory);

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
			imageInfo.imageView = imageIrradianceCube.imageView;
			imageInfo.sampler = imageIrradianceCube.sampler;

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
		{
			vk::DescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = uniformBuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(float);

			vk::WriteDescriptorSet descriptorWrite;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 2;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			device.getLogicalDevice().updateDescriptorSets(descriptorWrite, nullptr);
		}

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

		std::array<vk::DynamicState, 2> dynamicStateEnables = { vk::DynamicState::eViewport,vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = 2;

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.stageCount = static_cast<uint32_t>(prefilterShader->getShaderStages().size());
		pipelineInfo.pStages = prefilterShader->getShaderStages().data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.pDynamicState = &dynamicState;

		vk::Pipeline graphicsPipeline = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;

		vk::Image image;
		vk::ImageView view;
		vk::DeviceMemory memory;
		vk::Framebuffer framebuffer;
		{
			core::ImageInfoRequest imageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
			imageRequest.format = vk::Format::eR16G16B16A16Sfloat;
			imageRequest.width = CUBE_MAP_SIZE;
			imageRequest.height = CUBE_MAP_SIZE;
			imageRequest.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
			core::Utilities::createImage(imageRequest, image, memory);

			core::ImageViewInfoRequest imageViewRequest(device.getLogicalDevice(), image);
			imageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
			core::Utilities::createImageView(imageViewRequest, view);
		}
		
		//Create frame buffers for each face.
		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &view;
		framebufferInfo.width = CUBE_MAP_SIZE;
		framebufferInfo.height = CUBE_MAP_SIZE;
		framebufferInfo.layers = 1;

		framebuffer = device.getLogicalDevice().createFramebuffer(framebufferInfo);

		vk::UniqueCommandPool commandPool = device.getLogicalDevice().createCommandPoolUnique(poolInfo);

		vk::UniqueCommandBuffer commandImageBuffer = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		core::Utilities::transitionImageLayout(commandImageBuffer.get(), image, vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageAspectFlagBits::eColor);
		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandImageBuffer);


		// Begin a new single-time command buffer for the final layout transition
		vk::UniqueCommandBuffer transitionCommandBuffer = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		core::Utilities::transitionImageLayout(transitionCommandBuffer.get(), prefilterImage.image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageAspectFlagBits::eColor, 6, mipLevels);

		// End the layout transition command buffer
		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), transitionCommandBuffer);

		vk::UniqueCommandBuffer commandBuffer = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		vk::ClearValue clearColor{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} };

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea = vk::Rect2D({ 0, 0 }, { CUBE_MAP_SIZE, CUBE_MAP_SIZE });
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;


		commandBuffer.get().setScissor(0, 1, &scissor);
		float roughness = 0; 
		for (uint32_t m = 0; m < mipLevels; m++) {
			roughness = (float)m / (float)(mipLevels - 1);
			for (uint32_t face = 0; face < 6; face++)
			{
				viewport.width = static_cast<float>(CUBE_MAP_SIZE * std::pow(0.5f, m));
				viewport.height = static_cast<float>(CUBE_MAP_SIZE * std::pow(0.5f, m));
				commandBuffer.get().setViewport(0, 1, &viewport);

				updateRUniformBuffer(roughness, runiformBufferMemory);
				updateUniformBuffer(CameraViewMatrix::captureViews[face], CameraViewMatrix::captureProjection,
					uniformBufferMemory);

				// Begin render pass and render to the specific cube face
				commandBuffer.get().beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

				// Bind pipeline, descriptor sets, and draw commands
				commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
				commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet,
					{});

				// Bind vertex buffer
				vk::DeviceSize offsets[] = { 0 };
				commandBuffer.get().bindVertexBuffers(0, vertexBuffer, offsets);
				commandBuffer.get().draw(static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);

				commandBuffer.get().endRenderPass();

				// Ensure synchronization between rendering and copying by transitioning the image layout
				core::Utilities::transitionImageLayout(commandBuffer.get(), image,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::eTransferSrcOptimal,
					vk::ImageAspectFlagBits::eColor);

				// Set up the copy region for the transfer operation
				vk::ImageCopy copyRegion = {};
				copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				copyRegion.srcSubresource.baseArrayLayer = 0;
				copyRegion.srcSubresource.mipLevel = 0;
				copyRegion.srcSubresource.layerCount = 1;
				copyRegion.srcOffset = vk::Offset3D{ 0, 0, 0 };

				copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				copyRegion.dstSubresource.baseArrayLayer = face;
				copyRegion.dstSubresource.mipLevel = m;  // Set mip level to 0 for each face
				copyRegion.dstSubresource.layerCount = 1;
				copyRegion.dstOffset = vk::Offset3D{ 0, 0, 0 };

				copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
				copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
				copyRegion.extent.depth = 1;

				// Copy the image from the framebuffer to the cube map face
				commandBuffer.get().copyImage(image, vk::ImageLayout::eTransferSrcOptimal, prefilterImage.image,
					vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

				// Transition the image back to color attachment layout for the next face
				core::Utilities::transitionImageLayout(commandBuffer.get(), image,
					vk::ImageLayout::eTransferSrcOptimal,
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageAspectFlagBits::eColor);

			}
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

		// Begin a new single-time command buffer for the final layout transition
		vk::UniqueCommandBuffer transitionCommandBuffer2 = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		core::Utilities::transitionImageLayout(transitionCommandBuffer2.get(), prefilterImage.image,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageAspectFlagBits::eColor, 6, mipLevels);

		// End the layout transition command buffer
		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), transitionCommandBuffer2);

		device.getLogicalDevice().destroyFence(renderFence);

		device.getLogicalDevice().destroyBuffer(vertexBuffer);
		device.getLogicalDevice().freeMemory(vertexBufferMemory);

		device.getLogicalDevice().freeMemory(uniformBufferMemory);
		device.getLogicalDevice().freeMemory(runiformBufferMemory);

		device.getLogicalDevice().destroyFramebuffer(framebuffer);
		device.getLogicalDevice().freeMemory(memory);
		device.getLogicalDevice().destroyImageView(view);
		device.getLogicalDevice().destroyImage(image);

		device.getLogicalDevice().destroyRenderPass(renderPass);
		device.getLogicalDevice().destroyDescriptorPool(descriptorPool);
		device.getLogicalDevice().destroyDescriptorSetLayout(descriptorSetLayout);
		device.getLogicalDevice().destroyPipeline(graphicsPipeline);
		device.getLogicalDevice().destroyPipelineLayout(pipelineLayout);
	}

	void IBL::drawCube()
	{
		skyboxShader->readShader("../../resources/shaders/ibl/skybox.glsl");

		vk::AttachmentDescription colorAttachment{};
		colorAttachment.format = swapChain.getSwapchainImageFormat();
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

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

		renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

		//DEFINE THE VERTEX BUFFER
		std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptiones;
		vk::VertexInputBindingDescription vertexInputBindingDescription1;
		vertexInputBindingDescription1.binding = 0;
		vertexInputBindingDescription1.stride = sizeof(glm::vec3);
		vertexInputBindingDescription1.inputRate = vk::VertexInputRate::eVertex;
		vertexInputBindingDescriptiones.push_back(vertexInputBindingDescription1);

		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
		vertexInputAttributes.push_back({ 0, 0, vk::Format::eR32G32B32Sfloat, 0 });

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptiones.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

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
		core::BufferInfoRequest bufferRequest(device.getLogicalDevice(), device.getPhysicalDevice());
		bufferRequest.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufferRequest.properties = vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent;
		bufferRequest.size = sizeof(UniformBufferObject);
		core::Utilities::createBuffer(bufferRequest, uniformBuffer, uniformBufferMemory);

		
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
			imageInfo.imageView = imageIrradianceCube.imageView;
			imageInfo.sampler = imageIrradianceCube.sampler;

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
		viewport.width = static_cast<float>(swapChain.getSwapchainExtent().width);
		viewport.height = static_cast<float>(swapChain.getSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D(0, 0);
		scissor.extent = vk::Extent2D(swapChain.getSwapchainExtent().width, swapChain.getSwapchainExtent().height);

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
		pipelineLayout = device.getLogicalDevice().createPipelineLayout(pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.stageCount = static_cast<uint32_t>(skyboxShader->getShaderStages().size());
		pipelineInfo.pStages = skyboxShader->getShaderStages().data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		graphicsPipeline = device.getLogicalDevice().createGraphicsPipeline(nullptr, pipelineInfo).value;

		framebuffers.resize(offscreenResources.size());

		for (uint32_t i = 0; i < framebuffers.size(); i++) {
			vk::ImageView viewImage = offscreenResources[i].colorImageView;

			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &viewImage;
			framebufferInfo.width = swapChain.getSwapchainExtent().width;
			framebufferInfo.height = swapChain.getSwapchainExtent().height;
			framebufferInfo.layers = 1;

			framebuffers[i] = device.getLogicalDevice().createFramebuffer(framebufferInfo);
		}
	}

	void IBL::generateIrradianceCube()
	{
		core::ImageInfoRequest cubeMapImageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
		cubeMapImageRequest.format = vk::Format::eR16G16B16A16Sfloat;
		cubeMapImageRequest.layers = 6;
		cubeMapImageRequest.width = CUBE_MAP_SIZE;
		cubeMapImageRequest.height = CUBE_MAP_SIZE;
		cubeMapImageRequest.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eColorAttachment;
		cubeMapImageRequest.imageFlags = vk::ImageCreateFlagBits::eCubeCompatible;
		core::Utilities::createImage(cubeMapImageRequest, imageIrradianceCube.image,
			imageIrradianceCube.imageMemory);

		core::ImageViewInfoRequest cubeMapImageViewRequest(device.getLogicalDevice(), imageIrradianceCube.image);
		cubeMapImageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
		cubeMapImageViewRequest.layerCount = 6;
		cubeMapImageViewRequest.imageType = vk::ImageViewType::eCube;
		core::Utilities::createImageView(cubeMapImageViewRequest, imageIrradianceCube.imageView);

		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		imageIrradianceCube.sampler = device.getLogicalDevice().createSampler(samplerInfo);
		

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

		// Use subpass dependencies for layout transitions
		std::array<vk::SubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
		dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
			vk::AccessFlagBits::eColorAttachmentWrite;
		dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
		dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead |
			vk::AccessFlagBits::eColorAttachmentWrite;
		dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
		dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

		vk::RenderPassCreateInfo renderPassInfo;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subPass;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		vk::RenderPass renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);

		//DEFINE THE VERTEX BUFFER
		vk::VertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(glm::vec3);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
		vertexInputAttributes.push_back({ 0, 0, vk::Format::eR32G32B32Sfloat, 0 });

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
		bufferRequest.properties = vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent;
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

		vk::Image image;
		vk::ImageView view;
		vk::DeviceMemory memory;
		vk::Framebuffer framebuffer;

		core::ImageInfoRequest imageRequest(device.getLogicalDevice(), device.getPhysicalDevice());
		imageRequest.format = vk::Format::eR16G16B16A16Sfloat;
		imageRequest.width = CUBE_MAP_SIZE;
		imageRequest.height = CUBE_MAP_SIZE;
		imageRequest.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
		core::Utilities::createImage(imageRequest, image, memory);

		core::ImageViewInfoRequest imageViewRequest(device.getLogicalDevice(), image);
		imageViewRequest.format = vk::Format::eR16G16B16A16Sfloat;
		core::Utilities::createImageView(imageViewRequest, view);

		//Create frame buffers for each face.
		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &view;
		framebufferInfo.width = CUBE_MAP_SIZE;
		framebufferInfo.height = CUBE_MAP_SIZE;
		framebufferInfo.layers = 1;

		framebuffer = device.getLogicalDevice().createFramebuffer(framebufferInfo);

		//setUp command buffer
		vk::UniqueCommandPool commandPool = device.getLogicalDevice().createCommandPoolUnique(poolInfo);

		vk::UniqueCommandBuffer commandImageBuffer = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		core::Utilities::transitionImageLayout(commandImageBuffer.get(), image, vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageAspectFlagBits::eColor);
		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), commandImageBuffer);


		// Begin a new single-time command buffer for the final layout transition
		vk::UniqueCommandBuffer transitionCommandBuffer = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		core::Utilities::transitionImageLayout(transitionCommandBuffer.get(), imageIrradianceCube.image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageAspectFlagBits::eColor, 6);

		// End the layout transition command buffer
		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), transitionCommandBuffer);

		vk::UniqueCommandBuffer commandBuffer = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		vk::ClearValue clearColor{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} };

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea = vk::Rect2D({ 0, 0 }, { CUBE_MAP_SIZE, CUBE_MAP_SIZE });
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		//DRAW COMMAND 
		for (uint32_t face = 0; face < 6; ++face)
		{
			updateUniformBuffer(CameraViewMatrix::captureViews[face], CameraViewMatrix::captureProjection,
				uniformBufferMemory);

			// Begin render pass and render to the specific cube face
			commandBuffer.get().beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

			// Bind pipeline, descriptor sets, and draw commands
			commandBuffer.get().bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
			commandBuffer.get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet,
				{});

			// Bind vertex buffer
			vk::DeviceSize offsets[] = { 0 };
			commandBuffer.get().bindVertexBuffers(0, vertexBuffer, offsets);
			commandBuffer.get().draw(static_cast<uint32_t>(cubeVertices.size()), 1, 0, 0);

			commandBuffer.get().endRenderPass();

			// Ensure synchronization between rendering and copying by transitioning the image layout
			core::Utilities::transitionImageLayout(commandBuffer.get(), image,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::eTransferSrcOptimal,
				vk::ImageAspectFlagBits::eColor);

			// Set up the copy region for the transfer operation
			vk::ImageCopy copyRegion = {};
			copyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			copyRegion.srcSubresource.baseArrayLayer = 0;
			copyRegion.srcSubresource.mipLevel = 0;
			copyRegion.srcSubresource.layerCount = 1;
			copyRegion.srcOffset = vk::Offset3D{ 0, 0, 0 };

			copyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			copyRegion.dstSubresource.baseArrayLayer = face;
			copyRegion.dstSubresource.mipLevel = 0;  // Set mip level to 0 for each face
			copyRegion.dstSubresource.layerCount = 1;
			copyRegion.dstOffset = vk::Offset3D{ 0, 0, 0 };

			copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
			copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
			copyRegion.extent.depth = 1;

			// Copy the image from the framebuffer to the cube map face
			commandBuffer.get().copyImage(image, vk::ImageLayout::eTransferSrcOptimal, imageIrradianceCube.image,
				vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

			// Transition the image back to color attachment layout for the next face
			core::Utilities::transitionImageLayout(commandBuffer.get(), image,
				vk::ImageLayout::eTransferSrcOptimal,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageAspectFlagBits::eColor);

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

		// Begin a new single-time command buffer for the final layout transition
		vk::UniqueCommandBuffer transitionCommandBuffer2 = core::Utilities::beginSingleTimeCommands(
			device.getLogicalDevice(), commandPool.get());

		core::Utilities::transitionImageLayout(transitionCommandBuffer2.get(), imageIrradianceCube.image,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageAspectFlagBits::eColor, 6);

		// End the layout transition command buffer
		core::Utilities::endSingleTimeCommands(device.getGraphicsQueue(), transitionCommandBuffer2);

		//cleanUp
		device.getLogicalDevice().destroyFence(renderFence);

		device.getLogicalDevice().destroyBuffer(vertexBuffer);
		device.getLogicalDevice().freeMemory(vertexBufferMemory);
		device.getLogicalDevice().freeMemory(uniformBufferMemory);

		device.getLogicalDevice().destroyFramebuffer(framebuffer);
		device.getLogicalDevice().freeMemory(memory);
		device.getLogicalDevice().destroyImageView(view);
		device.getLogicalDevice().destroyImage(image);

		device.getLogicalDevice().destroyRenderPass(renderPass);
		device.getLogicalDevice().destroyDescriptorPool(descriptorPool);
		device.getLogicalDevice().destroyDescriptorSetLayout(descriptorSetLayout);
		device.getLogicalDevice().destroyPipeline(graphicsPipeline);
		device.getLogicalDevice().destroyPipelineLayout(pipelineLayout);
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

		vk::SamplerCreateInfo samplerInfo;
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.maxAnisotropy = 1.0;
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		brdfLUTImage.sampler = device.getLogicalDevice().createSampler(samplerInfo);

		brdfLUTShader->readShader("../../resources/shaders/ibl/brdf.glsl");

		//DEFINE THE VERTEX BUFFER
		vk::VertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof(QuadVertex);
		vertexInputBindingDescription.inputRate = vk::VertexInputRate::eVertex;

		std::vector<vk::VertexInputAttributeDescription> vertexInputAttributes;
		vertexInputAttributes.push_back({ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(QuadVertex, position) });
		vertexInputAttributes.push_back({ 1, 0, vk::Format::eR32G32Sfloat, offsetof(QuadVertex, texture) });

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
		colorAttachment.format = vk::Format::eR16G16Sfloat;
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
		dependency.srcStageMask = vk::PipelineStageFlagBits::eTransfer;
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

		vk::ClearValue clearColor{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} };
		vk::RenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea = vk::Rect2D({ 0, 0 }, { CUBE_MAP_SIZE, CUBE_MAP_SIZE });
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

		vk::DeviceSize offsets[] = { 0 };
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

		//cleanUp
		device.getLogicalDevice().destroyFramebuffer(framebuffer);
		device.getLogicalDevice().destroyFence(renderFence);

		device.getLogicalDevice().destroyBuffer(vertexBuffer);
		device.getLogicalDevice().freeMemory(vertexBufferMemory);

		device.getLogicalDevice().destroyRenderPass(renderPass);
		device.getLogicalDevice().destroyPipeline(graphicsPipeline);
		device.getLogicalDevice().destroyPipelineLayout(pipelineLayout);
	}

	void IBL::updateUniformBuffer(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
		const vk::DeviceMemory& uniformBufferMemory) const
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

	void IBL::updateRUniformBuffer(float r, const vk::DeviceMemory& uniformBufferMemory) const
	{

		void* data;
		vk::Result result = device.getLogicalDevice().mapMemory(uniformBufferMemory, 0, sizeof(float), {}, &data);
		if (result == vk::Result::eSuccess)
		{
			memcpy(data, &r, sizeof(float));
			device.getLogicalDevice().unmapMemory(uniformBufferMemory);
		}
	}

}
