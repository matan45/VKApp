#include "Mesh.hpp"
#include "print/EditorLogger.hpp"

#include <vector>
#include <fstream>
#include <bit>  // For std::bit_cast
#include <filesystem> 
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace types {
	void Mesh::loadFromFile(const importConfig::ImportFiles& file, std::string_view fileName, std::string_view location) const
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file.path.data(),
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			vfLogError("Failed to load Mesh file: {}", importer.GetErrorString());
			return;
		}
		//TODO also extract the animation each animation to single file 
		// and extract texture that embedded 
		processAssimpScene(scene, fileName, location);
	}

	void Mesh::saveToFile(std::string_view location, std::string_view fileName, const resource::MeshesData& meshesData) const
	{
		// Open the file in binary mode
		std::filesystem::path newFileLocation = std::filesystem::path(location) / (std::string(fileName) + "." + FileExtension::mesh);
		std::ofstream outFile(newFileLocation, std::ios::binary);

		if (!outFile) {
			vfLogError("Failed to open file for writing: ", newFileLocation.string());
			return;
		}

		// Write the header file type
		uint8_t headerFileType = static_cast<uint8_t>(meshesData.headerFileType);
		outFile.write(reinterpret_cast<const char*>(&headerFileType), sizeof(headerFileType));
		
		// Serialize the mesh data (this is just an example, adapt to your format)
		uint32_t majorVersion = std::bit_cast<uint32_t>(Version::major);
		uint32_t minorVersion = std::bit_cast<uint32_t>(Version::minor);
		uint32_t patchVersion = std::bit_cast<uint32_t>(Version::patch);
		outFile.write(std::bit_cast<const char*>(&majorVersion), sizeof(majorVersion));
		outFile.write(std::bit_cast<const char*>(&minorVersion), sizeof(minorVersion));
		outFile.write(std::bit_cast<const char*>(&patchVersion), sizeof(patchVersion));

		// Write the number of meshes
		outFile.write(std::bit_cast<const char*>(&meshesData.numberOfMeshes), sizeof(meshesData.numberOfMeshes));

		// Iterate through each mesh and write its data
		for (const auto& meshData : meshesData.meshes) {
			// Write the number of vertices
			uint32_t vertexCount = static_cast<uint32_t>(meshData.vertices.size());
			outFile.write(std::bit_cast<const char*>(&vertexCount), sizeof(vertexCount));

			// Write the vertex data (position, normal, and texCoords)
			outFile.write(std::bit_cast<const char*>(meshData.vertices.data()), vertexCount * sizeof(resource::Vertex));

			// Write the number of indices
			uint32_t indexCount = static_cast<uint32_t>(meshData.indices.size());
			outFile.write(std::bit_cast<const char*>(&indexCount), sizeof(indexCount));

			// Write the index data
			outFile.write(std::bit_cast<const char*>(meshData.indices.data()), indexCount * sizeof(uint32_t));
		}

		outFile.close();
	}

	void Mesh::processAssimpScene(const aiScene* scene, std::string_view fileName, std::string_view location) const
	{
		resource::MeshesData meshesData;
		meshesData.headerFileType = resource::FileType::MESH;
		meshesData.meshes.reserve(scene->mNumMeshes);
		meshesData.numberOfMeshes = scene->mNumMeshes;
		// Loop over each mesh in the scene
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			const aiMesh* assimpMesh = scene->mMeshes[i];
			resource::MeshData meshData;
			// Reserve space for vertices and indices
			meshData.vertices.reserve(assimpMesh->mNumVertices);
			meshData.indices.reserve(assimpMesh->mNumFaces * 3); // Assuming all faces are triangles

			// Extract vertex data (positions, normals, texCoords)
			for (unsigned int j = 0; j < assimpMesh->mNumVertices; ++j) {
				resource::Vertex vertex;

				// Extract position
				vertex.position = {
					assimpMesh->mVertices[j].x,
					assimpMesh->mVertices[j].y,
					assimpMesh->mVertices[j].z
				};

				// Extract normals (if they exist)
				if (assimpMesh->HasNormals()) {
					vertex.normal = {
						assimpMesh->mNormals[j].x,
						assimpMesh->mNormals[j].y,
						assimpMesh->mNormals[j].z
					};
				}
				else {
					vertex.normal = { 0.0f, 0.0f, 0.0f }; // Default normal if not available
				}

				// Extract texture coordinates (if they exist)
				if (assimpMesh->mTextureCoords[0]) { // Assimp allows multiple sets of UVs, we use the first one
					vertex.texCoords = {
						assimpMesh->mTextureCoords[0][j].x,
						assimpMesh->mTextureCoords[0][j].y
					};
				}
				else {
					vertex.texCoords = { 0.0f, 0.0f }; // Default texture coordinates if not available
				}

				// Add the vertex to the mesh data
				meshData.vertices.push_back(vertex);
			}

			// Extract indices
			for (unsigned int j = 0; j < assimpMesh->mNumFaces; ++j) {
				aiFace face = assimpMesh->mFaces[j];

				// Assimp ensures all faces are triangles when using aiProcess_Triangulate
				for (unsigned int k = 0; k < face.mNumIndices; ++k) {
					meshData.indices.push_back(face.mIndices[k]);
				}
			}
			meshesData.meshes.emplace_back(meshData);
		}
		saveToFile(location, fileName, meshesData);
	}
}
