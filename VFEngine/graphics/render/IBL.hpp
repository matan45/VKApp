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
    inline static std::vector<glm::vec3> cubeVertices = {
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

    inline static std::vector<glm::vec3> cubeNormals = {
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

   inline static std::vector<glm::vec2> cubeTextureCoords = {
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

    struct UniformBufferObject {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };

    struct ImageData
    {
        vk::Image cubeMapImage;
        vk::DeviceMemory cubeMapImageMemory;
        vk::ImageView cubeMapImageView;
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
        void updateUniformBuffer(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,const vk::DeviceMemory& uniformBufferMemory);
    };
}
