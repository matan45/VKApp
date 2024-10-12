#pragma once
#include <string>
#include "resource/Types.hpp"

namespace types {
	class Audio
	{
	public:
		void loadFromFile(std::string_view path) const;

	private:
		void loadOggFile(std::string_view path) const;
		void loadWavFile(std::string_view path) const;
		void loadMp3File(std::string_view path) const;
		void saveToFile(std::string_view path, const resource::AudioData& audioData) const;
	};
}

