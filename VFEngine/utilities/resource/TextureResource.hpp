#pragma once
#include <string>
#include "Types.hpp"

namespace resource {
	class TextureResource
	{
	private:
		inline static const size_t chunkSize = 1024 * 1024;  // Example chunk size for streaming
	public:
		static TextureData loadTexture(std::string_view path);
		static HDRData loadHDR(std::string_view path);
	};

}

