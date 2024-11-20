#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <stdexcept>

struct RGBE {
	uint8_t r, g, b, e; // Red, Green, Blue, Exponent
};


class HDRWriter {
public:

	bool writeHDR(const std::string& filepath, int width, int height, const std::vector<float>& pixels) {
		if (pixels.size() != width * height * 3) {
			std::cerr << "Pixel data size does not match image dimensions!" << std::endl;
			return false;
		}

		std::ofstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open file for writing: " << filepath << std::endl;
			return false;
		}

		// Write header
		file << "#?RADIANCE\n";
		file << "FORMAT=32-bit_rle_rgbe\n\n";
		file << "-Y " << height << " +X " << width << "\n";

		// Write pixel data
		for (int y = 0; y < height; ++y) {
			// Write scanline header
			uint8_t scanlineHeader[4] = { 2, 2, (uint8_t)(width >> 8), (uint8_t)(width & 0xFF) };
			file.write(reinterpret_cast<char*>(scanlineHeader), 4);

			for (int channel = 0; channel < 4; ++channel) {
				int x = 0;
				while (x < width) {
					int runLength = 1;
					while (x + runLength < width && runLength < 127 &&
						getChannel(pixels, width, y, x, channel) ==
						getChannel(pixels, width, y, x + runLength, channel)) {
						runLength++;
					}

					if (runLength > 1) { // RLE
						uint8_t value = getChannel(pixels, width, y, x, channel);
						file.put(128 + runLength);
						file.put(value);
					}
					else { // Raw data
						uint8_t value = getChannel(pixels, width, y, x, channel);
						file.put(1);
						file.put(value);
					}
					x += runLength;
				}
			}
		}

		file.close();
		return true;
	}


private:
	uint8_t getChannel(const std::vector<float>& pixels, int width, int y, int x, int channel) const {
		float r = pixels[(y * width + x) * 3 + 0];
		float g = pixels[(y * width + x) * 3 + 1];
		float b = pixels[(y * width + x) * 3 + 2];

		switch (channel) {
		case 0: return encodeRGBE(r, g, b).r; // Red
		case 1: return encodeRGBE(r, g, b).g; // Green
		case 2: return encodeRGBE(r, g, b).b; // Blue
		case 3: return encodeRGBE(r, g, b).e; // Exponent
		default: return 0;
		}
	}
	RGBE encodeRGBE(float r, float g, float b) const {
		float maxColor = std::max(r, std::max(g, b));
		if (maxColor < 1e-5f) return { 0, 0, 0, 0 };

		int e;
		float scale = std::frexp(maxColor, &e) * 256.0f / maxColor;

		return {
			(uint8_t)(r * scale),
			(uint8_t)(g * scale),
			(uint8_t)(b * scale),
			(uint8_t)(e + 128)
		};
	}
};



class HDRReader {
public:
	bool readHDR(const std::string& filepath, int& width, int& height, std::vector<float>& pixels) {
		std::ifstream file(filepath, std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open HDR file: " << filepath << std::endl;
			return false;
		}

		// Read and validate the header
		std::string line;
		if (!std::getline(file, line) || line != "#?RADIANCE") {
			std::cerr << "Invalid HDR file format: missing RADIANCE header." << std::endl;
			return false;
		}

		while (std::getline(file, line) && !line.empty()) {
			if (line.find("FORMAT=") == 0 && line != "FORMAT=32-bit_rle_rgbe") {
				std::cerr << "Unsupported HDR format: " << line << std::endl;
				return false;
			}
		}

		// Read image dimensions
		if (!std::getline(file, line)) {
			std::cerr << "Failed to read HDR dimensions." << std::endl;
			return false;
		}

		if (sscanf(line.c_str(), "-Y %d +X %d", &height, &width) != 2) {
			std::cerr << "Failed to parse HDR dimensions: " << line << std::endl;
			return false;
		}

		pixels.resize(width * height * 3); // Prepare the output pixel buffer

		// Read pixel data
		for (int y = 0; y < height; ++y) {
			// Read and validate scanline header
			uint8_t scanlineHeader[4];
			file.read(reinterpret_cast<char*>(scanlineHeader), 4);
			if (scanlineHeader[0] != 2 || scanlineHeader[1] != 2 ||
				(scanlineHeader[2] << 8 | scanlineHeader[3]) != width) {
				std::cerr << "Invalid or unsupported scanline header in HDR file." << std::endl;
				return false;
			}

			std::vector<uint8_t> scanline(width * 4);
			for (int channel = 0; channel < 4; ++channel) {
				int x = 0;
				while (x < width) {
					uint8_t count = file.get();
					if (count > 128) { // RLE-encoded run
						count -= 128;
						uint8_t value = file.get();
						for (int i = 0; i < count; ++i) {
							scanline[x * 4 + channel] = value;
							++x;
						}
					}
					else { // Raw data
						for (int i = 0; i < count; ++i) {
							scanline[x * 4 + channel] = file.get();
							++x;
						}
					}
				}
			}

			// Decode RGBE data
			for (int x = 0; x < width; ++x) {
				const RGBE& rgbe = *reinterpret_cast<const RGBE*>(&scanline[x * 4]);
				decodeRGBE(rgbe, pixels[(y * width + x) * 3 + 0],
					pixels[(y * width + x) * 3 + 1],
					pixels[(y * width + x) * 3 + 2]);
			}
		}

		return true;
	}

private:
	void decodeRGBE(const RGBE& rgbe, float& r, float& g, float& b) const {
		if (rgbe.e == 0) {
			r = g = b = 0.0f;
		}
		else {
			float scale = std::ldexp(1.0f, rgbe.e - 128 - 8); // 2^(e - 128) / 256
			r = rgbe.r * scale;
			g = rgbe.g * scale;
			b = rgbe.b * scale;
		}
	}
};
