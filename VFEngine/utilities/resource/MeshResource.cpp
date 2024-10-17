#include "MeshResource.hpp"
#include <fstream>
#include <iostream>
#include <bit>  // For std::bit_cast

namespace resource {

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
		inFile.read(std::bit_cast<char*>(meshData.vertices.data()), vertexCount * sizeof(resource::Vertex));

		// Read indices
		size_t indexCount = 0;
		inFile.read(std::bit_cast<char*>(&indexCount), sizeof(size_t));
		meshData.indices.resize(indexCount);  // Resize the vector to hold all indices
		inFile.read(std::bit_cast<char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));

		std::cout << "Mesh data successfully loaded from " << path << std::endl;

		return meshData;
	}
}

