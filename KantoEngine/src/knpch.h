#pragma once

//#include "Hazel/Core/PlatformDetection.h"

#ifdef KN_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Kanto/Core/Base.h"

#include "Kanto/Core/Log.h"

#include "Kanto/Debug/Instrumentor.h"

#include <Kanto/Core/Events/Event.h>

#include <Kanto/Core/Memory.h>

#ifdef KN_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
