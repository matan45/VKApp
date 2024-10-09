#pragma once
#include <string>

class StringUtil
{
public:
	static std::string wstringToUtf8(std::wstring_view wstr);
};

