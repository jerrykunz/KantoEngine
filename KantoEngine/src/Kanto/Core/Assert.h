#pragma once

#include "Kanto/Core/Base.h"
#include "Log.h"

#ifdef KN_PLATFORM_WINDOWS
#define KN_DEBUG_BREAK __debugbreak()
#elif defined(KN_COMPILER_CLANG)
#define KN_DEBUG_BREAK __builtin_debugtrap()
#else
#define KN_DEBUG_BREAK
#endif

#ifdef KN_DEBUG
#define KN_ENABLE_ASSERTS
#endif

#define KN_ENABLE_VERIFY

#ifdef KN_ENABLE_ASSERTS
#ifdef KN_COMPILER_CLANG
#define KN_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Core, "Assertion Failed", ##__VA_ARGS__)
#define KN_ASSERT_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Client, "Assertion Failed", ##__VA_ARGS__)
#else
//#define KN_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Core, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
//#define KN_ASSERT_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Client, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
#define KN_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Core, "Assertion Failed", ##__VA_ARGS__)
#define KN_ASSERT_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Client, "Assertion Failed", ##__VA_ARGS__)
#endif

#define KN_CORE_ASSERT(condition, ...) { if(!(condition)) { KN_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); KN_DEBUG_BREAK; } }
#define KN_ASSERT(condition, ...) { if(!(condition)) { KN_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); KN_DEBUG_BREAK; } }
#else
#define KN_CORE_ASSERT(condition, ...)
#define KN_ASSERT(condition, ...)
#endif

#ifdef KN_ENABLE_VERIFY
#ifdef KN_COMPILER_CLANG
#define KN_CORE_VERIFY_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Core, "Verify Failed", ##__VA_ARGS__)
#define KN_VERIFY_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Client, "Verify Failed", ##__VA_ARGS__)
#else
//#define KN_CORE_VERIFY_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Core, "Verify Failed" __VA_OPT__(,) __VA_ARGS__)
//#define KN_VERIFY_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Client, "Verify Failed" __VA_OPT__(,) __VA_ARGS__)
#define KN_CORE_VERIFY_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Core, "Verify Failed", ##__VA_ARGS__)
#define KN_VERIFY_MESSAGE_INTERNAL(...)  ::Kanto::Log::PrintAssertMessage(::Kanto::Log::Type::Client, "Verify Failed", ##__VA_ARGS__)
#endif

#define KN_CORE_VERIFY(condition, ...) { if(!(condition)) { KN_CORE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); KN_DEBUG_BREAK; } }
#define KN_VERIFY(condition, ...) { if(!(condition)) { KN_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); KN_DEBUG_BREAK; } }
#else
#define KN_CORE_VERIFY(condition, ...)
#define KN_VERIFY(condition, ...)
#endif
