#include "Import.hpp"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace controller {

	void Import::importFiles(const std::vector<std::string>& paths)
	{
		// Initialize the Assimp Importer
		Assimp::Importer importer;

		// Load a 3D model file (e.g., model.obj) with post-processing flags
		const aiScene* scene = importer.ReadFile("../../resources/barrel.obj",
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

		// Check if the import was successful
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "Error importing model: " << importer.GetErrorString() << std::endl;
			return;
		}

		// Print some basic information about the model's meshes
		printMeshInfo(scene);
	}

	void Import::printMeshInfo(const aiScene* scene) {
		if (!scene) {
			std::cerr << "Error: No scene data available.\n";
			return;
		}

		// Loop over all the meshes in the loaded scene
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
			const aiMesh* mesh = scene->mMeshes[i];
			std::cout << "Mesh " << i << ": " << mesh->mName.C_Str() << std::endl;
			std::cout << "Number of vertices: " << mesh->mNumVertices << std::endl;

			// Print out positions of the first few vertices
			std::cout << "First few vertex positions:\n";
			for (unsigned int v = 0; v < std::min(mesh->mNumVertices, 5u); ++v) {
				const aiVector3D& pos = mesh->mVertices[v];
				std::cout << "  Vertex " << v << ": (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
			}
		}
	}


}
