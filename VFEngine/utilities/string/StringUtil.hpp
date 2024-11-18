#pragma once
#include <string>
#include <vector>
#include <shobjidl.h>

class StringUtil
{
public:
    static std::string wstringToUtf8(std::wstring_view wstr);
    static std::string WideStringToString(PWSTR wideStr);
    static std::wstring utf8ToWstring(const std::string& utf8Str);
    static std::string toLower(const std::string& str);
    static size_t findFirstNotOf(const std::vector<unsigned char>& data, const std::string& chars);
};
