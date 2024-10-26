#pragma once
#include <string>
#include "Types.hpp"

namespace resource {
	class MeshResource
	{
	private:
		inline static const size_t chunkSize = 1024 * 1024;  // Define a chunk size to read in portions
	public:
		static MeshesData loadMesh(std::string_view path);
	};
}

