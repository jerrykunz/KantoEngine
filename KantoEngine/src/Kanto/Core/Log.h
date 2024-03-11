#pragma once

#include "Kanto/Core/Base.h"
#include "Kanto/Core/LogCustomFormatters.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/fmt/fmt.h"

#include <map>

#define KN_ASSERT_MESSAGE_BOX (!KN_DIST && KN_PLATFORM_WINDOWS)

#if KN_ASSERT_MESSAGE_BOX
#ifdef KN_PLATFORM_WINDOWS
#include <Windows.h>
#endif
#endif

namespace Kanto {

	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() { return s_EditorConsoleLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, const char* format, Args&&... args);

		template<typename... Args>
		static void PrintAssertMessage(Log::Type type, std::string_view prefix, fmt::format_string<Args...> s, Args&&... args);
		static void PrintAssertMessage(Log::Type type, std::string_view prefix);

	public:
		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
			case Level::Trace: return "Trace";
			case Level::Info:  return "Info";
			case Level::Warn:  return "Warn";
			case Level::Error: return "Error";
			case Level::Fatal: return "Fatal";
			}
			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;

			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_EditorConsoleLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;
	};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core logging
#define KN_CORE_TRACE_TAG(tag, ...) ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Trace, tag, __VA_ARGS__)
#define KN_CORE_INFO_TAG(tag, ...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Info, tag, __VA_ARGS__)
#define KN_CORE_WARN_TAG(tag, ...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Warn, tag, __VA_ARGS__)
#define KN_CORE_ERROR_TAG(tag, ...) ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Error, tag, __VA_ARGS__)
#define KN_CORE_FATAL_TAG(tag, ...) ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Fatal, tag, __VA_ARGS__)

// Client logging
#define KN_TRACE_TAG(tag, ...) ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Trace, tag, __VA_ARGS__)
#define KN_INFO_TAG(tag, ...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Info, tag, __VA_ARGS__)
#define KN_WARN_TAG(tag, ...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Warn, tag, __VA_ARGS__)
#define KN_ERROR_TAG(tag, ...) ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Error, tag, __VA_ARGS__)
#define KN_FATAL_TAG(tag, ...) ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define KN_CORE_TRACE(...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Trace, "", __VA_ARGS__)
#define KN_CORE_INFO(...)   ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Info, "", __VA_ARGS__)
#define KN_CORE_WARN(...)   ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Warn, "", __VA_ARGS__)
#define KN_CORE_ERROR(...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Error, "", __VA_ARGS__)
#define KN_CORE_FATAL(...)  ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Core, ::Kanto::Log::Level::Fatal, "", __VA_ARGS__)

// Client Logging
#define KN_TRACE(...)   ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Trace, "", __VA_ARGS__)
#define KN_INFO(...)    ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Info, "", __VA_ARGS__)
#define KN_WARN(...)    ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Warn, "", __VA_ARGS__)
#define KN_ERROR(...)   ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Error, "", __VA_ARGS__)
#define KN_FATAL(...)   ::Kanto::Log::PrintMessage(::Kanto::Log::Type::Client, ::Kanto::Log::Level::Fatal, "", __VA_ARGS__)

// Editor Console Logging Macros
#define KN_CONSOLE_LOG_TRACE(...)   Kanto::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
#define KN_CONSOLE_LOG_INFO(...)    Kanto::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
#define KN_CONSOLE_LOG_WARN(...)    Kanto::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
#define KN_CONSOLE_LOG_ERROR(...)   Kanto::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
#define KN_CONSOLE_LOG_FATAL(...)   Kanto::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)

namespace Kanto
{

	template<typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag, const char* format, Args&&... args)
	{
		auto detail = s_EnabledTags[std::string(tag)];
		if (detail.Enabled && detail.LevelFilter <= level)
		{
			auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();

			std::string logString = tag.empty() ? "{0}{1}" : "[{0}] {1}";
			switch (level)
			{
			case Level::Trace:
				logger->info(fmt::runtime(logString), tag, fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
				break;
			case Level::Info:
				logger->info(fmt::runtime(logString), tag, fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
				break;
			case Level::Warn:
				logger->warn(fmt::runtime(logString), tag, fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
				break;
			case Level::Error:
				logger->error(fmt::runtime(logString), tag, fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
				break;
			case Level::Fatal:
				logger->critical(fmt::runtime(logString), tag, fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
				break;
			}
		}
	}

	//could be runtime like above function, but I guess this is fine
	template<typename... Args>
	void Log::PrintAssertMessage(Log::Type type, std::string_view prefix, fmt::format_string<Args...> s, Args&&... args)
	{
		auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}: {1}", prefix, fmt::format(s, std::forward<Args>(args)...));

#if KN_ASSERT_MESSAGE_BOX
		std::string message = fmt::format(s, std::forward<Args>(args)...);
		MessageBoxA(nullptr, message.c_str(), "Kanto Assert", MB_OK | MB_ICONERROR);
#endif
	}

	inline void Log::PrintAssertMessage(Log::Type type, std::string_view prefix)
	{
		auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}", prefix);
#if KN_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, "No message :(", "Kanto Assert", MB_OK | MB_ICONERROR);
#endif
	}
}
