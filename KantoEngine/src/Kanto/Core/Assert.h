#pragma once

#include "Kanto/Core/Base.h"
#include "Kanto/Core/Log.h"
#include <filesystem>

#ifdef KN_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message



//msg is the "Assertion failed: {0}", further arguments go inside {0}, {1}, {2], etc...
//When using KN_ASSERT, the type is '_' and the resulting string from KN##type##ERROR is 'KN_ERROR'
//When using KN_CORE_ASSERT, the type is '_CORE_'  and the resulting string from KN##type##ERROR is 'KN_CORE_ERROR'. Both of these are defined in Log.h
#define KN_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { KN##type##ERROR(msg, __VA_ARGS__); KN_DEBUGBREAK(); } }

//just calls to macro above, with or without messages
#define KN_INTERNAL_ASSERT_WITH_MSG(type, check, ...) KN_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define KN_INTERNAL_ASSERT_NO_MSG(type, check) KN_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", KN_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

//called by KN_INTERNAL_ASSERT_GET_MACRO with 3 or 4 arguments
//If there are only 3 arguments, no messages is included. In this case the third argument is KN_INTERNAL_ASSERT_NO_MSG
//If there are 4 arguments, the third argument is KN_INTERNAL_ASSERT_WITH_MSG
//Arguments beyond that (...) don't matter
#define KN_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro

//gets the macro KN_INTERNAL_ASSERT_WITH_MSG or KN_INTERNAL_ASSERT_NO_MSG, based on number of arguments given, using KN_INTERNAL_ASSERT_GET_MACRO_NAME
#define KN_INTERNAL_ASSERT_GET_MACRO(...) KN_EXPAND_MACRO( KN_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, KN_INTERNAL_ASSERT_WITH_MSG, KN_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define KN_ASSERT(...) KN_EXPAND_MACRO( KN_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )

//(...) accepts any number of arguments
//KN_EXPAND_MACRO is used to ensure everything is defined correctly, because KN_INTERNAL_ASSERT_GET_MACRO uses macros to define itself as well
//KN_INTERNAL_ASSERT_GET_MACRO gets the correct macro based on argument number (KN_INTERNAL_ASSERT_WITH_MSG or KN_INTERNAL_ASSERT_NO_MSG)
//(_CORE_, __VA_ARGS__) are the arguments that are given to the macro returned by KN_INTERNAL_ASSERT_GET_MACRO
#define KN_CORE_ASSERT(...) KN_EXPAND_MACRO( KN_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define KN_ASSERT(...)
#define KN_CORE_ASSERT(...)
#endif
