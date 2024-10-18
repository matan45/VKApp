#include "AudioResource.hpp"
#include <fstream>
#include <iostream>
#include <bit>  // For std::bit_cast

namespace resource {

	AudioData AudioResource::loadAudio(std::string_view path)
	{
		resource::AudioData audioData;

		// Open the file in binary mode
		std::ifstream inFile(path.data(), std::ios::binary);
		if (!inFile) {
			std::cerr << "Failed to open file for reading: " << path << std::endl;
			return audioData; // Return an empty audioData on failure
		}

		// Read version
		inFile.read(std::bit_cast<char*>(&audioData.version.major), sizeof(audioData.version.major));
		inFile.read(std::bit_cast<char*>(&audioData.version.minor), sizeof(audioData.version.minor));
		inFile.read(std::bit_cast<char*>(&audioData.version.patch), sizeof(audioData.version.patch));

		// Read audio metadata: sample rate, channels, frames, total duration
		inFile.read(std::bit_cast<char*>(&audioData.sampleRate), sizeof(audioData.sampleRate));
		inFile.read(std::bit_cast<char*>(&audioData.channels), sizeof(audioData.channels));
		inFile.read(std::bit_cast<char*>(&audioData.frames), sizeof(audioData.frames));
		inFile.read(std::bit_cast<char*>(&audioData.totalDurationInSeconds), sizeof(audioData.totalDurationInSeconds));

		// Read the size of the raw audio data
		uint32_t dataSize;
		inFile.read(std::bit_cast<char*>(&dataSize), sizeof(dataSize));

		size_t bytesRemaining = dataSize;

		audioData.data.reserve(dataSize);  // Reserve space for the entire buffer if needed
		size_t currentOffset = 0;

		while (bytesRemaining > 0) {
			size_t bytesToRead = std::min(chunkSize, bytesRemaining);

			// Read chunk
			std::vector<short> buffer(bytesToRead / sizeof(short));
			inFile.read(std::bit_cast<char*>(buffer.data()), bytesToRead);

			// Process or play this chunk (e.g., feeding it to an audio buffer)
			audioData.data.insert(audioData.data.end(), buffer.begin(), buffer.end());

			currentOffset += bytesToRead;
			bytesRemaining -= bytesToRead;

			// Optionally, play the chunk in an audio engine here
			// playAudioChunk(buffer);

			std::cout << "Loaded " << currentOffset << " / " << dataSize << " bytes of audio data." << std::endl;
		}

		std::cout << "Texture data successfully loaded from " << path << std::endl;
		return audioData;
	}
}

