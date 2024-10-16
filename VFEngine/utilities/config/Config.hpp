#pragma once
#include <cstdint>
#include <string>

struct Version
{
	uint32_t major = 0;
	uint32_t minor = 0;
	uint32_t patch = 1;
};

struct FileExtension
{
	inline static const std::string textrue = "vfImage";
	inline static const std::string audio = "vfAudio";
	inline static const std::string mesh = "vfMesh";
	inline static const std::string animation = "vfAnim";
};

