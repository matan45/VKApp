#pragma once

#ifdef APIENTRY
#undef APIENTRY
#endif

#include "spdlog/spdlog.h"
#include <source_location>
#include <format>


// Usage
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define loggerInfo(...) util::infoLog(__FILENAME__,__LINE__,__VA_ARGS__)
#define loggerWarning(...) util::warningLog(__FILENAME__,__LINE__,__VA_ARGS__)
#define loggerError(...) util::infoError(__FILENAME__,__LINE__,__VA_ARGS__)
#define loggerAssert(condition,...) util::infoAssert(__FILENAME__,__LINE__,condition,__VA_ARGS__)


namespace util {

	template<typename... Args>
	using format_string_t = fmt::format_string<Args...>;

	template<typename... Args>
	inline void infoLog(const char* filename, int line, format_string_t<Args...> fmt, Args &&... args)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		printf("%s (line %d) INFO: \n", filename, line);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);

		spdlog::info(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void warningLog(const char* filename, int line, format_string_t<Args...> fmt, Args &&... args)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED);
		printf("%s (line %d) WARNING: \n", filename, line);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);

		spdlog::warn(fmt, std::forward<Args>(args)...);
	}


	template<typename... Args>
	inline void infoError(const char* filename, int line, format_string_t<Args...> fmt, Args &&... args)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		printf("%s (line %d) ERROR: \n", filename, line);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);

		spdlog::error(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void infoAssert(const char* filename, int line, bool condition, format_string_t<Args...> fmt, Args&&... args)
	{
		if (!condition) { // Changed from `if (condition)` to `if (!condition)`
			const std::string fullErrorMessage = std::format(
				"Critical Assertion Failure\n\n{} (line {}) Assertion Failure:\n{}",
				filename, line, std::format(fmt, std::forward<Args>(args)...)
			);

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
			printf("%s", fullErrorMessage.c_str());
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);

			spdlog::error(fmt, std::forward<Args>(args)...);

			MessageBoxA(nullptr, fullErrorMessage.c_str(), "Critical Assertion Failure", MB_ICONEXCLAMATION | MB_OK);
			exit(-1);
		}
	}

}

