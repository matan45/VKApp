#include "ImguiRender.hpp"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

#include "../core/Device.hpp"
#include "../core/SwapChain.hpp"
#include "../core/CommandPool.hpp"
#include "../window/Window.hpp"
#include "log/Logger.hpp"

namespace imgui {

	ImguiRender::ImguiRender(core::Device& device, core::SwapChain& swapChain, core::CommandPool& commandPool, window::Window& window) :
		device{ device }, swapChain{ swapChain }, commandPool{ commandPool }, window{ window }
	{

	}

	void ImguiRender::init()
	{
		createDescriptorPool();
		createRenderPass();
		createFrameBuffers();


		// Setup Platform/Renderer back ends
		ImGui_ImplGlfw_InitForVulkan(window.getWindowPtr(), true);
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = device.getInstance();
		initInfo.PhysicalDevice = device.getPhysicalDevice();
		initInfo.Device = device.getLogicalDevice();
		initInfo.QueueFamily = device.getQueueFamilyIndices().graphicsAndComputeFamily.value();
		initInfo.Queue = device.getGraphicsQueue();
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = imGuiDescriptorPool;
		initInfo.MinImageCount = swapChain.getImageCount();
		initInfo.ImageCount = swapChain.getImageCount();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = VK_NULL_HANDLE;
		ImGui_ImplVulkan_Init(&initInfo);

		/*vk::CommandBuffer commandBuffer = renderer::Uitil::beginSingleTimeCommands(device, commandPool.getCommand());
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		renderer::Uitil::endSingleTimeCommands(device, commandPool.getCommand(), commandBuffer);*/

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

		theme();
	}

	void ImguiRender::cleanUp() const
	{
		for (auto framebuffer : imGuiFrameBuffers) {
			device.getLogicalDevice().destroyFramebuffer(framebuffer);
		}

		// Destroy the graphics pipeline, pipeline layout, and render pass
		device.getLogicalDevice().destroyRenderPass(imGuiRenderPass);
		device.getLogicalDevice().destroyDescriptorPool(imGuiDescriptorPool);
	}

	void ImguiRender::recreate(uint32_t width, uint32_t height)
	{
		for (auto framebuffer : imGuiFrameBuffers) {
			device.getLogicalDevice().destroyFramebuffer(framebuffer);
		}

		// Destroy the graphics pipeline, pipeline layout, and render pass
		device.getLogicalDevice().destroyRenderPass(imGuiRenderPass);

		createRenderPass();
		createFrameBuffers();
	}

	void ImguiRender::render(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const
	{
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();

		vk::ClearValue clearColor = { std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f} };

		vk::RenderPassBeginInfo renderPassinfo = {};
		renderPassinfo.renderPass = imGuiRenderPass;
		renderPassinfo.framebuffer = imGuiFrameBuffers[imageIndex];
		renderPassinfo.renderArea.extent.width = swapChain.getSwapchainExtent().width;
		renderPassinfo.renderArea.extent.height = swapChain.getSwapchainExtent().height;
		renderPassinfo.clearValueCount = 1;
		renderPassinfo.pClearValues = &clearColor;

		commandBuffer.beginRenderPass(renderPassinfo, vk::SubpassContents::eInline);

		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

		commandBuffer.endRenderPass();

		drawData->Clear();
	}

	void ImguiRender::theme() const
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
		style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
		style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
		style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		style.GrabRounding = style.FrameRounding = 2.3f;
	}

	void ImguiRender::createRenderPass()
	{
		vk::AttachmentDescription colorAttachment{};
		colorAttachment.format = swapChain.getSwapchainImageFormat();
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

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

		imGuiRenderPass = device.getLogicalDevice().createRenderPass(renderPassInfo);
	}

	void ImguiRender::createDescriptorPool()
	{
		std::vector<vk::DescriptorPoolSize> poolSizes =
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer , 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment , 1000 }
		};

		vk::DescriptorPoolCreateInfo poolInfo = {};
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		poolInfo.maxSets = static_cast <uint32_t>(1000 * poolSizes.size());
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		try
		{
			imGuiDescriptorPool = device.getLogicalDevice().createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError& err)
		{
			loggerError("failed to create DescriptorPool {}", err.what());
		}
	}

	void ImguiRender::createFrameBuffers()
	{
		imGuiFrameBuffers.resize(swapChain.getImageCount());

		for (uint32_t i = 0; i < imGuiFrameBuffers.size(); i++) {
			vk::ImageView viewImage = swapChain.getSwapchainImageView(i);

			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = imGuiRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &viewImage;
			framebufferInfo.width = swapChain.getSwapchainExtent().width;
			framebufferInfo.height = swapChain.getSwapchainExtent().height;
			framebufferInfo.layers = 1;

			imGuiFrameBuffers[i] = device.getLogicalDevice().createFramebuffer(framebufferInfo);
		}
	}

}