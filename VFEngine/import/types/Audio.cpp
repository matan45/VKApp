#include "Audio.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <bit>  // For std::bit_cast
#include <filesystem> 

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>
#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>
#include <stb_vorbis.c>


namespace types {
	void Audio::loadFromFile(std::string_view path, std::string_view fileName, std::string_view extension, std::string_view location) const
	{
		if (extension == "ogg") {
			loadOggFile(path, fileName, location);
		}
		else if (extension == "wav") {
			loadWavFile(path, fileName, location);
		}
		else if (extension == "mp3") {
			loadMp3File(path, fileName, location);
		}
	}

	void Audio::loadOggFile(std::string_view path, std::string_view fileName, std::string_view location) const
	{
		resource::AudioData audioData;

		// Open and load Ogg Vorbis file using stb_vorbis
		int error;
		stb_vorbis* vorbis = stb_vorbis_open_filename(path.data(), &error, nullptr);
		if (!vorbis) {
			std::cerr << "Failed to load Ogg Vorbis file: " << path << std::endl;
			return;
		}

		// Retrieve file information
		stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		audioData.sampleRate = info.sample_rate;
		audioData.channels = info.channels;

		// Get the total number of samples
		int totalSamples = stb_vorbis_stream_length_in_samples(vorbis) * info.channels;
		audioData.frames = stb_vorbis_stream_length_in_samples(vorbis);
		audioData.totalDurationInSeconds = static_cast<uint32_t>(stb_vorbis_stream_length_in_seconds(vorbis));

		// Resize the data buffer and read samples
		audioData.data.resize(totalSamples);
		stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, audioData.data.data(), totalSamples);

		saveToFile(location, fileName, audioData);

		// Cleanup
		stb_vorbis_close(vorbis);

		std::cout << "Ogg Vorbis file loaded successfully!" << std::endl;
	}

	void Audio::loadWavFile(std::string_view path, std::string_view fileName, std::string_view location) const
	{
		resource::AudioData audioData;

		// Open and load WAV file using dr_wav
		drwav wav;
		if (!drwav_init_file(&wav, path.data(), nullptr)) {
			std::cerr << "Failed to load WAV file: " << path << std::endl;
			return ;
		}

		// Set up the AudioData structure
		audioData.version = { 1, 0, 0 }; // Version
		audioData.sampleRate = wav.sampleRate;
		audioData.channels = wav.channels;
		audioData.frames = static_cast<uint32_t>(wav.totalPCMFrameCount);
		audioData.totalDurationInSeconds = static_cast<uint32_t>(wav.totalPCMFrameCount / wav.sampleRate);

		// Load WAV data into the vector (16-bit signed samples)
		audioData.data.resize(wav.totalPCMFrameCount * wav.channels);
		drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, audioData.data.data());

		saveToFile(location, fileName, audioData);

		// Cleanup
		drwav_uninit(&wav);

		std::cout << "WAV file loaded successfully!" << std::endl;
	}

	void Audio::loadMp3File(std::string_view path, std::string_view fileName, std::string_view location) const
	{
		resource::AudioData audioData;

		// Open and load MP3 file using dr_mp3
		drmp3 mp3;
		if (!drmp3_init_file(&mp3, path.data(), nullptr)) {
			std::cerr << "Failed to load MP3 file: " << path << std::endl;
			return ;
		}

		// Set up the AudioData structure
		audioData.version = { 1, 0, 0 }; // Version
		audioData.sampleRate = mp3.sampleRate;
		audioData.channels = mp3.channels;

		// Read the MP3 data into a temporary buffer
		drmp3_uint64 totalFrames = drmp3_get_pcm_frame_count(&mp3);
		audioData.frames = static_cast<uint32_t>(totalFrames);
		audioData.totalDurationInSeconds = static_cast<uint32_t>(totalFrames / mp3.sampleRate);

		// Resize the audio data buffer and read into it
		audioData.data.resize(totalFrames * mp3.channels);
		drmp3_read_pcm_frames_s16(&mp3, totalFrames, audioData.data.data());

		saveToFile(location, fileName, audioData);

		// Cleanup
		drmp3_uninit(&mp3);

		std::cout << "MP3 file loaded successfully!" << std::endl;
	}

	void Audio::saveToFile(std::string_view location, std::string_view fileName, const resource::AudioData& audioData) const
	{
		// Open the file in binary mode
		std::filesystem::path newFileLocation = std::filesystem::path(location) / (std::string(fileName) + "." + FileExtension::audio);
		std::ofstream outFile(newFileLocation, std::ios::binary);
		if (!outFile) {
			std::cerr << "Failed to open file for writing: " << newFileLocation << std::endl;
			return;
		}

		// Write version
		outFile.write(std::bit_cast<const char*>(&audioData.version.major), sizeof(audioData.version.major));
		outFile.write(std::bit_cast<const char*>(&audioData.version.minor), sizeof(audioData.version.minor));
		outFile.write(std::bit_cast<const char*>(&audioData.version.patch), sizeof(audioData.version.patch));

		// Write sample rate, channels, frames, and total duration
		outFile.write(std::bit_cast<const char*>(&audioData.sampleRate), sizeof(audioData.sampleRate));
		outFile.write(std::bit_cast<const char*>(&audioData.channels), sizeof(audioData.channels));
		outFile.write(std::bit_cast<const char*>(&audioData.frames), sizeof(audioData.frames));
		outFile.write(std::bit_cast<const char*>(&audioData.totalDurationInSeconds), sizeof(audioData.totalDurationInSeconds));

		// Write audio data size and the raw audio data
		auto dataSize = static_cast<uint32_t>(audioData.data.size());
		outFile.write(std::bit_cast<const char*>(&dataSize), sizeof(dataSize));
		outFile.write(std::bit_cast<const char*>(audioData.data.data()), dataSize * sizeof(short));

		outFile.close();

		std::cout << "Audio data saved to " << newFileLocation << std::endl;
	}
}

