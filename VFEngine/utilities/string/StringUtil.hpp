#pragma once
#include <string>
#include <shobjidl.h> 

class StringUtil
{
public:
	static std::string wstringToUtf8(std::wstring_view wstr);
	static std::string WideStringToString(PWSTR wideStr);
	static std::wstring utf8ToWstring(const std::string& utf8Str);

};

