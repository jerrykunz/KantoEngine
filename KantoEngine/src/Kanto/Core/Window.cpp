#include "knpch.h"
#include "Kanto/Core/Window.h"

#ifdef KN_PLATFORM_WINDOWS
#include "Platform/GLFW/GlfwWindow.h"
#endif

namespace Kanto
{
	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef KN_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		KN_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}
