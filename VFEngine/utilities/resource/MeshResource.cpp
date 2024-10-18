#include "MeshResource.hpp"
#include <fstream>
#include <iostream>
#include <bit>  // For std::bit_cast

namespace resource {
	//TODO swap the the logs
	MeshData MeshResource::loadMesh(std::string_view path)
	{
		resource::MeshData meshData;
		// Open the file in binary mode
		std::ifstream inFile(path.data(), std::ios::binary);
		if (!inFile) {
			std::cerr << "Failed to open file for reading: " << path << std::endl;
			return meshData; // Return an empty audioData on failure
		}

		// Read version
		inFile.read(std::bit_cast<char*>(&meshData.version.major), sizeof(meshData.version.major));
		inFile.read(std::bit_cast<char*>(&meshData.version.minor), sizeof(meshData.version.minor));
		inFile.read(std::bit_cast<char*>(&meshData.version.patch), sizeof(meshData.version.patch));

		// Read vertices
		size_t vertexCount = 0;
		inFile.read(std::bit_cast<char*>(&vertexCount), sizeof(size_t));
		meshData.vertices.resize(vertexCount);  // Resize the vector to hold all vertices

		size_t verticesProcessed = 0;
		while (verticesProcessed < vertexCount) {
			size_t chunkToRead = std::min(chunkSize, vertexCount - verticesProcessed);
			inFile.read(std::bit_cast<char*>(meshData.vertices.data() + verticesProcessed),
				chunkToRead * sizeof(resource::Vertex));
			verticesProcessed += chunkToRead;

			// Check for stream failure
			if (inFile.fail()) {
				throw std::runtime_error("Failed to read vertex data from file.");
			}
		}

		// Read indices
		size_t indexCount = 0;
		inFile.read(std::bit_cast<char*>(&indexCount), sizeof(size_t));
		meshData.indices.resize(indexCount);  // Resize the vector to hold all indices
		// Read the index data in chunks
		size_t indicesProcessed = 0;
		while (indicesProcessed < indexCount) {
			size_t chunkToRead = std::min(chunkSize, indexCount - indicesProcessed);
			inFile.read(std::bit_cast<char*>(meshData.indices.data() + indicesProcessed),
				chunkToRead * sizeof(uint32_t));
			indicesProcessed += chunkToRead;

			if (inFile.fail()) {
				throw std::runtime_error("Failed to read index data from file.");
			}
		}

		std::cout << "Mesh data successfully loaded from " << path << std::endl;

		return meshData;
	}
}

