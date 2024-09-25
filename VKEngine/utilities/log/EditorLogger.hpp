#pragma once
#ifdef APIENTRY
#undef APIENTRY
#endif

#include <chrono>
#include <iomanip>
#include <sstream>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/bundled/core.h"  // fmt library used by spdlog


#define eloggerInfo(...) util::infoLog(__VA_ARGS__)
#define eloggerWarning(...) util::warningLog(__VA_ARGS__)
#define eloggerError(...) util::infoError(__VA_ARGS__)


namespace util {

	inline std::string getCurrentTime() {
		// Get the current time
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);

		// Convert it to a tm structure
		std::tm local_time;
		localtime_s(&local_time, &now_time);

		// Format the time as [HH:MM:SS]
		std::ostringstream oss;
		oss << std::put_time(&local_time, "[%H:%M:%S] ");
		return oss.str();
	}

	// Buffer to store log messages for ImGui console
	std::vector<std::string> imguiConsoleBuffer;

	// Append log message to ImGui buffer
	inline void appendToImGuiConsole(const std::string& message) {
		imguiConsoleBuffer.push_back(message);
	}

	// Helper function to set console text color (Windows-specific)
	inline void setConsoleColor(WORD color) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
	}

	// Helper function to reset console text color to default
	inline void resetConsoleColor() {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0F);  // Reset to normal
	}

	template<typename... Args>
	using format_string_t = fmt::format_string<Args...>;

	// Info logging function (without file name and line)
	template<typename... Args>
	inline void infoLog(format_string_t<Args...> fmt, Args&&... args) {
		std::string currentTime = getCurrentTime();

		// Format log message with just the message
		std::string formattedMessage = fmt::format(fmt, std::forward<Args>(args)...);
		std::string fullMessage = fmt::format("{}INFO: {}", currentTime, formattedMessage);

		// Set console color to green for info
		setConsoleColor(FOREGROUND_GREEN);
		printf("%s\n", fullMessage.c_str());
		resetConsoleColor();

		// Log using spdlog
		spdlog::info(formattedMessage);

		// Append the log with a timestamp to ImGui buffer
		appendToImGuiConsole(fullMessage);
	}

	// Warning logging function (without file name and line)
	template<typename... Args>
	inline void warningLog(format_string_t<Args...> fmt, Args&&... args) {
		std::string currentTime = getCurrentTime();

		// Format log message
		std::string formattedMessage = fmt::format(fmt, std::forward<Args>(args)...);
		std::string fullMessage = fmt::format("{}WARNING: {}", currentTime, formattedMessage);

		// Set console color to yellow for warning
		setConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED);  // Yellow text for warning
		printf("%s\n", fullMessage.c_str());
		resetConsoleColor();

		// Log using spdlog
		spdlog::warn(formattedMessage);

		// Append the log with a timestamp to ImGui buffer
		appendToImGuiConsole(fullMessage);
	}

	// Error logging function (without file name and line)
	template<typename... Args>
	inline void infoError(format_string_t<Args...> fmt, Args&&... args) {
		std::string currentTime = getCurrentTime();

		// Format log message
		std::string formattedMessage = fmt::format(fmt, std::forward<Args>(args)...);
		std::string fullMessage = fmt::format("{}ERROR: {}", currentTime, formattedMessage);

		// Set console color to red for error
		setConsoleColor(FOREGROUND_RED);
		printf("%s\n", fullMessage.c_str());
		resetConsoleColor();

		// Log using spdlog
		spdlog::error(formattedMessage);

		// Append the log with a timestamp to ImGui buffer
		appendToImGuiConsole(fullMessage);
	}

}  // namespace util
