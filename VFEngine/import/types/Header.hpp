#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>

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

