#pragma once
#include <string>
#include "Types.hpp"


namespace resource {
	class AudioResource
	{
	public:
		static AudioData loadAudio(std::string_view path);
	};

}


