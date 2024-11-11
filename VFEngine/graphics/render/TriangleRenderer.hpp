#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../core/OffScreen.hpp"

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

		core::OffscreenResources& offscreenResources;

	public:
		explicit TriangleRenderer(core::Device& device, core::SwapChain& swapChain, core::OffscreenResources& offscreenResources);
		~TriangleRenderer()=default;

		void init();
		void recreate();
		void cleanUp() const;

		void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

	private:
		void createVertexBuffer();
		void createGraphicsPipeline();
		void createFrameBuffers();
		void createRenderPass();

		void bindingDescription();
		void attributeDescriptions();
	};

}

