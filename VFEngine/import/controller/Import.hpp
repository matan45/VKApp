#pragma once
#include <vector>
#include <string>

struct aiScene;

namespace controller {
	class Import
	{
	public:
		static void importFiles(const std::vector<std::string>& paths);
		static void printMeshInfo(const aiScene* scene);
	};
}


