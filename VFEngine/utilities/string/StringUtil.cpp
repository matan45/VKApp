#include "StringUtil.hpp"
#include <Windows.h>
#include <stdexcept>
#include <algorithm>
#include "../print/EditorLogger.hpp"

 std::string StringUtil::wstringToUtf8(std::wstring_view wstr)
{
	if(wstr.empty())
	{
		return "";
	}

	const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	if (size_needed <= 0)
	{
		vfLogError("WideCharToMultiByte() failed: {}", std::to_string(size_needed));
		return std::string();
	}

	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), result.data(), size_needed, nullptr, nullptr);
	return result;
}

 std::string StringUtil::WideStringToString(PWSTR wideStr)
 {
	 int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
	 std::string strTo(sizeNeeded, 0);
	 WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &strTo[0], sizeNeeded, NULL, NULL);
	 return strTo;
 }

 std::wstring StringUtil::utf8ToWstring(const std::string& utf8Str)
 {
	 int wideSize = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
	 std::wstring wideString(wideSize, 0);
	 MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideString[0], wideSize);
	 return wideString;
 }

 std::string StringUtil::toLower(const std::string& str)
 {
	 std::string lowerStr = str;
	 std::ranges::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
		 [](unsigned char c) { return std::tolower(c); });
	 return lowerStr;
 }
