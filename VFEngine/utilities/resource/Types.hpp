#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstdint>
#include <string>
#include "../config/Config.hpp"

namespace resource
{
    enum class ShaderType :uint8_t
    {
        VERTEX,
        FRAGMENT,
        COMPUTE,
        GEOMETRY,
        TESS_CONTROL,
        TESS_EVALUATION,
        UNKNOWN
    };

    enum class FileType : uint8_t
    {
        SHADER,
        TEXTURE,
        MESH,
        ANIMATION,
        HDR,
        AUDIO,
        UNKNOWN
    };

    struct TextureData
    {
        FileType headerFileType;
        Version version;
        uint32_t width;
        uint32_t height;
        std::string textureFormat;
        std::vector<unsigned char> textureData;
    };

    struct HDRData
    {
        FileType headerFileType;
        Version version;
        uint32_t width;
        uint32_t height;
        std::string textureFormat;
        std::vector<unsigned char> textureData;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct MeshData
    {
        FileType headerFileType;
        Version version;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };

    struct Bone
    {
        std::string name; // Name of the bone
        glm::mat4 offsetMatrix; // Inverse Bind Pose Matrix (Bone's offset matrix)
        std::vector<std::pair<uint32_t, float>> weights; // Vertex index and weight pairs
    };

    struct Keyframe
    {
        float time;
        glm::vec3 position;
        glm::quat rotation; // Quaternions (w, x, y, z)
        glm::vec3 scale;
    };

    struct BoneAnimation
    {
        std::string boneName;
        std::vector<Keyframe> positionKeys;
        std::vector<Keyframe> rotationKeys;
        std::vector<Keyframe> scalingKeys;
    };

    struct AnimationData
    {
        FileType headerFileType;
        Version version;
        float duration;
        float ticksPerSecond;
        uint32_t numBones;
        std::vector<BoneAnimation> boneAnimations;
        std::vector<Bone> bones;
    };

    struct AudioData
    {
        FileType headerFileType;
        Version version;
        uint32_t totalDurationInSeconds;
        uint32_t channels;
        uint32_t sampleRate;
        uint32_t frames;
        std::vector<short> data;
    };
}
