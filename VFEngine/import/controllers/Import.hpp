#pragma once
#include <vector>
#include <string>

namespace controllers {
	class Import
	{
	public:
		static void importFiles(const std::vector<std::string>& paths);
	private:
		static void processPath(const std::string& path);
	};
}


