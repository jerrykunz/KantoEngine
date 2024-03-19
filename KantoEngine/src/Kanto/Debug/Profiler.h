#pragma once

#define KN_ENABLE_PROFILING !KN_DIST

#if KN_ENABLE_PROFILING 
#include <tracy/Tracy.hpp>
#endif

#if KN_ENABLE_PROFILING
#define KN_PROFILE_MARK_FRAME			FrameMark;
// NOTE(Peter): Use KN_PROFILE_FUNC ONLY at the top of a function
//				Use KN_PROFILE_SCOPE / KN_PROFILE_SCOPE_DYNAMIC for an inner scope
//#define KN_PROFILE_FUNC(...)			ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
#define KN_PROFILE_FUNC(...)	ZoneScoped##N(__VA_ARGS__) //replacement, not sure if this is gonna work --J
#define KN_PROFILE_SCOPE(...)			KN_PROFILE_FUNC(__VA_ARGS__)
#define KN_PROFILE_SCOPE_DYNAMIC(NAME)  ZoneScoped; ZoneName(NAME, strlen(NAME))
#define KN_PROFILE_THREAD(...)          tracy::SetThreadName(__VA_ARGS__)
#else
#define KN_PROFILE_MARK_FRAME
#define KN_PROFILE_FUNC(...)
#define KN_PROFILE_SCOPE(...)
#define KN_PROFILE_SCOPE_DYNAMIC(NAME)
#define KN_PROFILE_THREAD(...)
#endif
