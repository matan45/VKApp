#include "IBL.hpp"
#include "../core/Utilities.hpp"
#include "print/Logger.hpp"
#include "../core/Device.hpp"
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
		cubeMapImageRequest.format = vk::Format::eR8G8B8A8Srgb;
		cubeMapImageRequest.layers = 6;
		cubeMapImageRequest.width = CUBE_MAP_SIZE;
		cubeMapImageRequest.height = CUBE_MAP_SIZE;
		cubeMapImageRequest.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled |
			vk::ImageUsageFlagBits::eColorAttachment;
		cubeMapImageRequest.imageFlags = vk::ImageCreateFlagBits::eCubeCompatible;
		core::Utilities::createImage(cubeMapImageRequest, imageIrradianceCube.image,
			imageIrradianceCube.imageMemory);

		core::ImageViewInfoRequest cubeMapImageViewRequest(device.getLogicalDevice(), imageIrradianceCube.image);
		cubeMapImageViewRequest.format = vk::Format::eR8G8B8A8Srgb;
		cubeMapImageViewRequest.layerCount = 6;
		cubeMapImageViewRequest.imageType = vk::ImageViewType::eCube;
		core::Utilities::createImageView(cubeMapImageViewRequest, imageIrradianceCube.imageView);

		shaderIrradianceCube->readShader("../../resources/shaders/ibl/equirectangular_convolution.glsl");

		//SET UP RENDER PASS
		vk::AttachmentDescription colorAttachment;
		colorAttachment.format = vk::Format::eR8G8B8A8Srgb;
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
		imageRequest.format = vk::Format::eR8G8B8A8Srgb;
		imageRequest.width = CUBE_MAP_SIZE;
		imageRequest.height = CUBE_MAP_SIZE;
		imageRequest.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
		core::Utilities::createImage(imageRequest, image, memory);

		core::ImageViewInfoRequest imageViewRequest(device.getLogicalDevice(), image);
		imageViewRequest.format = vk::Format::eR8G8B8A8Srgb;
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

}
