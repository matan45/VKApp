#pragma once
#include <string>
#include <shobjidl.h> 
#include <vector>
#include <string_view>

namespace nfd {
	class FileDialog
	{
	public:
		// Constructor: Initializes COM.
		explicit FileDialog();
		~FileDialog();
		std::string openFileDialog(const std::vector<std::pair<std::wstring, std::wstring>>& fileTypes) const;
		std::vector<std::string> multiSelectFileDialog(const std::vector<std::pair<std::wstring, std::wstring>>& fileTypes) const;
	};

}

