#pragma once

#include <memory>

//refs are here in Base.h
//#include "Ref.h"

namespace Kanto {

	void InitializeCore();
	void ShutdownCore();

}

#if !defined(KN_PLATFORM_WINDOWS) && !defined(KN_PLATFORM_LINUX)
#error Unknown platform.
#endif

#define BIT(x) (1u << x)

#if defined(__clang__)
#define KN_COMPILER_CLANG
#elif defined(_MSC_VER)
#define KN_COMPILER_MSVC
#endif

#define KN_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#ifdef KN_COMPILER_MSVC
#define KN_FORCE_INLINE __forceinline
#elif defined(KN_COMPILER_CLANG)
#define KN_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define KN_FORCE_INLINE inline
#endif

#include "Assert.h"

namespace Kanto {

	// Pointer wrappers
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

	template<typename T>
	using WeakRef = std::weak_ptr<T>;


	using byte = uint8_t;

	/** A simple wrapper for std::atomic_flag to avoid confusing
		function names usage. The object owning it can still be
		default copyable, but the copied flag is going to be reset.
	*/
	struct AtomicFlag
	{
		KN_FORCE_INLINE void SetDirty() { flag.clear(); }
		KN_FORCE_INLINE bool CheckAndResetIfDirty() { return !flag.test_and_set(); }

		explicit AtomicFlag() noexcept { flag.test_and_set(); }
		AtomicFlag(const AtomicFlag&) noexcept {}
		AtomicFlag& operator=(const AtomicFlag&) noexcept { return *this; }
		AtomicFlag(AtomicFlag&&) noexcept {};
		AtomicFlag& operator=(AtomicFlag&&) noexcept { return *this; }

	private:
		std::atomic_flag flag;
	};

	struct Flag
	{
		KN_FORCE_INLINE void SetDirty() noexcept { flag = true; }
		KN_FORCE_INLINE bool CheckAndResetIfDirty() noexcept
		{
			if (flag)
				return !(flag = !flag);
			else
				return false;
		}

		KN_FORCE_INLINE bool IsDirty() const noexcept { return flag; }

	private:
		bool flag = false;
	};

}
