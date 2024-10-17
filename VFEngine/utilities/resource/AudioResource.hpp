#pragma once
#include <string>
#include "Types.hpp"


namespace resource {
	class AudioResource
	{
	private:
		inline static const size_t chunkSize = 1024 * 1024;  // Example chunk size for streaming
	public:
		static AudioData loadAudio(std::string_view path);
	};

}


