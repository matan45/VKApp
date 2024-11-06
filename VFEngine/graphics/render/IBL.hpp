#pragma once
#include <vector>
#include "../core/OffScreen.hpp"
#include "../core/Shader.hpp"
#include "../core/Texture.hpp"

namespace core
{
    class Device;
    class SwapChain;
}

namespace render
{
    inline static const std::vector<glm::vec3> cubeVertices = {
        {-1.0f, 1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},

        {-1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f}, {-1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f},

        {1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},

        {-1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f},

        {-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f},

        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}
    };

    inline static const std::vector<glm::vec3> cubeNormals = {
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},

        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},

        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},

        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},

        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},

        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}
    };

    inline static const std::vector<glm::vec2> cubeTextureCoords = {
        {0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
        {1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},

        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f},

        {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},

        {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f},
        {0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},

        {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f},
        {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f},

        {0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 1.0f},
        {1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}
    };

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec2 texture;
    };

    inline static const std::vector<QuadVertex> quad = {
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Vertex 0
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // Vertex 1
        {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // Vertex 2

        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Vertex 0 (repeated)
        {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // Vertex 2 (repeated)
        {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}} // Vertex 3
    };

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };


    struct ImageData
    {
        vk::Image image;
        vk::DeviceMemory imageMemory;
        vk::ImageView imageView;
    };

    struct CameraViewMatrix
    {
        inline static const std::array<glm::mat4, 6> captureViews = {
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
            glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
        };
        inline static const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    };


    class IBL
    {
    private:
        core::Device& device;
        core::SwapChain& swapChain;
        std::vector<core::OffscreenResources>& offscreenResources;
        vk::CommandPoolCreateInfo poolInfo;
        std::shared_ptr<core::Texture> hdrTexture;

        static constexpr uint32_t CUBE_MAP_SIZE = 512;
        
        ImageData imageIrradianceCube;
        std::shared_ptr<core::Shader> shaderIrradianceCube;

        ImageData brdfLUTImage;
        std::shared_ptr<core::Shader> brdfLUTShader;

        ImageData prefilterImage;
        std::shared_ptr<core::Shader> prefilterShader;

        std::shared_ptr<core::Shader> skyboxShader;
        vk::RenderPass renderPass;
        vk::Pipeline graphicsPipeline;
        std::vector<vk::Framebuffer> framebuffers;
        vk::Buffer vertexBuffer;
		vk::Buffer uniformBuffer;
		vk::DeviceMemory uniformBufferMemory;
        vk::PipelineLayout pipelineLayout;
        vk::DescriptorSet descriptorSet;

    public:
        explicit IBL(core::Device& device, core::SwapChain& swapChain,
                     std::vector<core::OffscreenResources>& offscreenResources);
        ~IBL() = default;

        void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

        void init(std::string_view path);
        void recreate();
        void cleanUp() const;

    private:
        void generateIrradianceCube();
        void generateBRDFLUT();
        void generatePrefilteredCube();
        void drawCube();
        void updateUniformBuffer(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                 const vk::DeviceMemory& uniformBufferMemory) const;
    };
}
