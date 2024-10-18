#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <glm/glm.hpp>
#include "../imguiPass/OffScreen.hpp"

namespace core {
	class Device;
	class SwapChain;
	class Shader;
}


namespace render {

	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;
	};


	class TriangleRenderer
	{
	private:
		core::Device& device;
		core::SwapChain& swapChain;

		core::Shader* triangle{ nullptr };

		vk::PipelineLayout pipelineLayout;
		vk::Pipeline graphicsPipeline;

		vk::Buffer vertexBuffer;
		vk::DeviceMemory vertexBufferMemory;

		vk::VertexInputBindingDescription vertexInputBindingDescription;
		std::vector <vk::VertexInputAttributeDescription> vertexInputAttributes;

		vk::RenderPass renderPass;
		std::vector<vk::Framebuffer> framebuffers;

		std::vector<imguiPass::OffscreenResources>& offscreenResources;

	public:
		explicit TriangleRenderer(core::Device& device, core::SwapChain& swapChain, std::vector<imguiPass::OffscreenResources>& offscreenResources);
		~TriangleRenderer()=default;

		void init();
		void recreate();
		void cleanUp() const;

		void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) const;

	private:
		void createVertexBuffer();
		void createGraphicsPipeline();
		void createFramebuffers();
		void createRenderPass();

		void bindingDescription();
		void attributeDescriptions();
	};

}

