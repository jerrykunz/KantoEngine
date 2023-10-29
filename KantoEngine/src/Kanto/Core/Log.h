#pragma once

#include "Kanto/Core/Base.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Kanto 
{

	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

// Core log macros
#define KN_CORE_TRACE(...)    ::Kanto::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KN_CORE_INFO(...)     ::Kanto::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KN_CORE_WARN(...)     ::Kanto::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KN_CORE_ERROR(...)    ::Kanto::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KN_CORE_CRITICAL(...) ::Kanto::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define KN_TRACE(...)         ::Kanto::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KN_INFO(...)          ::Kanto::Log::GetClientLogger()->info(__VA_ARGS__)
#define KN_WARN(...)          ::Kanto::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KN_ERROR(...)         ::Kanto::Log::GetClientLogger()->error(__VA_ARGS__)
#define KN_CRITICAL(...)      ::Kanto::Log::GetClientLogger()->critical(__VA_ARGS__)



//DELETE
//// Core log macros
//#define KN_CORE_TRACE(...)    
//#define KN_CORE_INFO(...)     
//#define KN_CORE_WARN(...)     
//#define KN_CORE_ERROR(...)    
//#define KN_CORE_CRITICAL(...) 
//
//// Client log macros
//#define KN_TRACE(...)         
//#define KN_INFO(...)          
//#define KN_WARN(...)          
//#define KN_ERROR(...)         
//#define KN_CRITICAL(...)      
