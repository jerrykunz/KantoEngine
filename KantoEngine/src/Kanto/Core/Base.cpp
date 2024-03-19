#include "knpch.h"
#include "Base.h"

#include "Log.h"
#include "Memory.h"

#define KANTO_BUILD_ID "v0.1a"

namespace Kanto
{

	void InitializeCore()
	{
		//Allocator::Init();
		Log::Init();

		KN_CORE_TRACE_TAG("Core", "Kanto Engine {}", KANTO_BUILD_ID);
		KN_CORE_TRACE_TAG("Core", "Initializing...");
	}

	void ShutdownCore()
	{
		KN_CORE_TRACE_TAG("Core", "Shutting down...");

		Log::Shutdown();
	}

}
