#pragma once
#include <vector>
#include "../core/OffScreen.hpp"
#include "../core/Shader.hpp"
#include "../core/Texture.hpp"

namespace core
{
    class Device;
    class SwapChain;
    class Shader;
}

namespace render
{
    // Cube vertices for a unit cube.
    std::vector<glm::vec3> cubeVertices = {
        // Positions
        {-1.0f, -1.0f, -1.0f},
        {1.0f, -1.0f, -1.0f},
        {1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, -1.0f},
        {-1.0f, -1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, 1.0f},
    };

    // Indices for drawing the cube with triangles.
    std::vector<uint32_t> cubeIndices = {
        0, 1, 2, 2, 3, 0, // -Z face
        4, 5, 6, 6, 7, 4, // +Z face
        0, 4, 7, 7, 3, 0, // -X face
        1, 5, 6, 6, 2, 1, // +X face
        3, 7, 6, 6, 2, 3, // +Y face
        0, 4, 5, 5, 1, 0 // -Y face
    };

    class IBL
    {
    private:
        core::Device& device;
        core::SwapChain& swapChain;
        std::vector<core::OffscreenResources>& offscreenResources;
        std::shared_ptr<core::Shader> shader;
        std::shared_ptr<core::Texture> hdrTexture;

    public:
        explicit IBL(core::Device& device, core::SwapChain& swapChain,
                     std::vector<core::OffscreenResources>& offscreenResources);
        ~IBL() = default;

        void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex) const;

        void init(std::string_view path);
        void recreate();
        void cleanUp() const;

    private:
        void generatePrefilteredCube();
        void generateIrradianceCube();
        void generateBRDFLUT();
    };
}
