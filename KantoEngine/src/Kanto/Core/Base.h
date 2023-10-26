#pragma once

#include "Kanto/Core/PlatformDetection.h"

#include <memory>

#ifdef KN_DEBUG
	#if defined(KN_PLATFORM_WINDOWS)
		#define KN_DEBUGBREAK() __debugbreak()
	#elif defined(KN_PLATFORM_LINUX)
		#include <signal.h>
		#define KN_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define KN_ENABLE_ASSERTS
#else
	#define KN_DEBUGBREAK()
#endif

//really complex, used when calling macros that call macros, that call macros
//basically it defines when the macro is "expanded", AKA when the value is set 
#define KN_EXPAND_MACRO(x) x

//can turn the value into a const char* string, doesn't work for everything. Function names don't work for example.
#define KN_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

//creates lamda functions out of class member functions, must be called in the same class
#define KN_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Kanto {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#include "Kanto/Core/Log.h"
#include "Kanto/Core/Assert.h"
