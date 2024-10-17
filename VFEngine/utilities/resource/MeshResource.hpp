#pragma once
#include <string>
#include "Types.hpp"

namespace resource {
	class MeshResource
	{
	public:
		static MeshData loadMesh(std::string_view path);
	};
}

