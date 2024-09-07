#pragma once

#include "spdlog/include/spdlog/spdlog.h"
#include <source_location>


namespace util {

	// Use std::source_location to automatically capture file and line information
	template<typename... Args>
	inline void infoLog(std::format_string<Args...> fmt, Args&&... args,
		std::source_location location = std::source_location::current())
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
		spdlog::info("[{}:{}] INFO: {}", location.file_name(), location.line(), std::format(fmt, std::forward<Args>(args)...));
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	template<typename... Args>
	inline void warningLog(std::format_string<Args...> fmt, Args&&... args,
		std::source_location location = std::source_location::current())
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED);
		spdlog::warn("[{}:{}] WARNING: {}", location.file_name(), location.line(), std::format(fmt, std::forward<Args>(args)...));
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	template<typename... Args>
	inline void errorLog(std::format_string<Args...> fmt, Args&&... args,
		std::source_location location = std::source_location::current())
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
		spdlog::error("[{}:{}] ERROR: {}", location.file_name(), location.line(), std::format(fmt, std::forward<Args>(args)...));
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	template<typename... Args>
	inline void assertLog(bool condition, std::format_string<Args...> fmt, Args&&... args,
		std::source_location location = std::source_location::current())
	{
		if (!condition) {
			const std::string fullErrorMessage = std::format("Critical Assertion Failure\n\n{} (line {}) Assertion Failure:\n{}",
				location.file_name(), location.line(), std::format(fmt, std::forward<Args>(args)...));

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
			spdlog::error("{}", fullErrorMessage);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

			MessageBoxA(nullptr, fullErrorMessage.c_str(), "Critical Assertion Failure", MB_ICONEXCLAMATION | MB_OK);
			exit(-1);
		}
	}

}

// Usage
#define loggerInfo(...) util::infoLog(__VA_ARGS__)
#define loggerWarning(...) util::warningLog(__VA_ARGS__)
#define loggerError(...) util::errorLog(__VA_ARGS__)
#define loggerAssert(condition, ...) util::assertLog(condition, __VA_ARGS__)


