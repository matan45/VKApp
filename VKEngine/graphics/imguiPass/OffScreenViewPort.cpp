#include "OffScreenViewPort.hpp"
#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/CommandPool.hpp"
#include "../core/Utilities.hpp"
#include "../render/RenderPassHandler.hpp"


namespace imguiPass {
	OffScreenViewPort::OffScreenViewPort(core::Device& device, core::SwapChain& swapChain) :device{ device }
		, swapChain{ swapChain }
	{
		commadPool = new core::CommandPool(device, swapChain);
	}

	OffScreenViewPort::~OffScreenViewPort()
	{
		delete commadPool;
	}
	void OffScreenViewPort::init()
	{
		renderPassHandler = new render::RenderPassHandler(device, swapChain);
		renderPassHandler->init();

		createSampler();
		createRenderPass();
		createOffscreenResources();
		createDescriptorSet();
		createSyncObjects();
	}

	vk::DescriptorSet OffScreenViewPort::render()
	{
		return vk::DescriptorSet();
	}

	void OffScreenViewPort::cleanUp() const
	{
		device.getLogicalDevice().waitIdle();

		renderPassHandler->cleanUp();

		commadPool->cleanUp();

		device.getLogicalDevice().destroySampler(sampler);

		device.getLogicalDevice().destroySemaphore(imageAvailableSemaphore);
		device.getLogicalDevice().destroySemaphore(renderFinishedSemaphore);
		device.getLogicalDevice().destroyFence(renderFence);

		for (auto const& resources : offscreenResources) {
			device.getLogicalDevice().destroyImageView(resources.colorImageView);
			device.getLogicalDevice().destroyImageView(resources.depthImageView);
			device.getLogicalDevice().destroyImage(resources.colorImage);
			device.getLogicalDevice().destroyImage(resources.depthImage);
			device.getLogicalDevice().freeMemory(resources.colorImageMemory);
			device.getLogicalDevice().freeMemory(resources.depthImageMemory);
			device.getLogicalDevice().destroyFramebuffer(resources.framebuffer);
		}

		device.getLogicalDevice().destroyRenderPass(renderPass);
	}

	void OffScreenViewPort::draw(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		renderPassHandler->draw(commandBuffer, imageIndex);
	}

	void OffScreenViewPort::createOffscreenResources()
	{
		vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;
		vk::Format depthFormat = swapChain.getSwapchainDepthStencilFormat();

		for (size_t i = 0; i < swapChain.getImageCount(); i++) {
			OffscreenResources resources;

			// Create color image for off-screen rendering
			createImage(colorFormat, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, resources.colorImage, resources.colorImageMemory);
			createImageView(resources.colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, resources.colorImageView);

			// Create depth image
			createImage(depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment, resources.depthImage, resources.depthImageMemory);
			createImageView(resources.depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, resources.depthImageView);

			// Create framebuffer
			createFramebuffer(resources.framebuffer);

			// Store resources
			offscreenResources.push_back(std::move(resources));
		}
	}

	void OffScreenViewPort::createImage(vk::Format format, vk::ImageUsageFlags usage, vk::Image& image, vk::DeviceMemory& deviceMemory) const
	{
		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = swapChain.getSwapchainExtent().width;
		imageInfo.extent.height = swapChain.getSwapchainExtent().height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;

		image = device.getLogicalDevice().createImage(imageInfo);

		vk::MemoryRequirements memRequirements = device.getLogicalDevice().getImageMemoryRequirements(image);
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = core::Utilities::findMemoryType(device.getPhysicalDevice(), memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		deviceMemory = device.getLogicalDevice().allocateMemory(allocInfo);
		device.getLogicalDevice().bindImageMemory(image, deviceMemory, 0);
	}

	void OffScreenViewPort::createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageView& imageView) const
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = image;
		viewInfo.viewType = vk::ImageViewType::e2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		imageView = device.getLogicalDevice().createImageView(viewInfo);
	}

	void OffScreenViewPort::createRenderPass()
	{
		vk::AttachmentDescription colorAttachment{};
		colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;  // Ready for ImGui

		vk::AttachmentDescription depthAttachment{};
		depthAttachment.format = swapChain.getSwapchainDepthStencilFormat();
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass{};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		vk::RenderPassCreateInfo renderPassInfo{};
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		renderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);
	}

	void OffScreenViewPort::createDescriptorSet()
	{
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		for (auto& resources : offscreenResources) {
			resources.descriptorSet = device.getLogicalDevice().allocateDescriptorSets(allocInfo)[0];

			vk::DescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageInfo.imageView = resources.colorImageView;
			imageInfo.sampler = sampler;

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite.dstSet = resources.descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			device.getLogicalDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
		}
	}

	void OffScreenViewPort::createFramebuffer(vk::Framebuffer& framebuffer)
	{
		std::array<vk::ImageView, 2> attachments = {
			offscreenResources.back().colorImageView,
			offscreenResources.back().depthImageView
		};

		vk::FramebufferCreateInfo framebufferInfo{};
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChain.getSwapchainExtent().width;
		framebufferInfo.height = swapChain.getSwapchainExtent().height;
		framebufferInfo.layers = 1;

		framebuffer = device.getLogicalDevice().createFramebuffer(framebufferInfo);
	}

	void OffScreenViewPort::createSyncObjects()
	{
		// Create semaphores for synchronization
		vk::SemaphoreCreateInfo semaphoreInfo{};
		imageAvailableSemaphore = device.getLogicalDevice().createSemaphore(semaphoreInfo);
		renderFinishedSemaphore = device.getLogicalDevice().createSemaphore(semaphoreInfo);

		// Create a fence for GPU-CPU synchronization
		vk::FenceCreateInfo fenceInfo{};
		renderFence = device.getLogicalDevice().createFence(fenceInfo);
	}

	void OffScreenViewPort::createSampler()
	{
		vk::SamplerCreateInfo samplerInfo{};

		// Filtering mode (how the texture is sampled)
		samplerInfo.magFilter = vk::Filter::eLinear;  // Magnification filter
		samplerInfo.minFilter = vk::Filter::eLinear;  // Minification filter

		// Addressing mode (what happens when sampling outside [0, 1] UV range)
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;

		vk::PhysicalDeviceProperties properties = device.getPhysicalDevice().getProperties();

		// Retrieve the maximum anisotropy level supported by the device
		float maxAnisotropy = properties.limits.maxSamplerAnisotropy;

		// Mipmapping options
		samplerInfo.anisotropyEnable = VK_TRUE;  // Enable anisotropic filtering if supported
		samplerInfo.maxAnisotropy = maxAnisotropy;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;  // Use black for border sampling
		samplerInfo.unnormalizedCoordinates = VK_FALSE;  // Use normalized coordinates [0, 1]
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = vk::CompareOp::eAlways;

		// Mipmap settings
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;  // Linear mipmap interpolation
		samplerInfo.mipLodBias = 0.0f;  // No mipmap level of detail bias
		samplerInfo.minLod = 0.0f;  // Minimum LOD (mipmap level)
		samplerInfo.maxLod = 0.0f;  // Maximum LOD (since we're not using mipmaps, keep this at 0)

		// Create the sampler
		sampler = device.getLogicalDevice().createSampler(samplerInfo);
	}

}
