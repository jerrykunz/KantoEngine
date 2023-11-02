#include "knpch.h"
#include "Kanto/Platform/OpenGL/OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Kanto {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		KN_CORE_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		KN_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KN_CORE_ASSERT(status, "Failed to initialize Glad!");

		//spdlog sublibrary fmt cannot handle glubytes that come from glGetString, so let's cast them as const char*
		KN_CORE_INFO("OpenGL Info:");
		KN_CORE_INFO("  Vendor: {0}", (const char*)(glGetString(GL_VENDOR)));
		KN_CORE_INFO("  Renderer: {0}", (const char*)(glGetString(GL_RENDERER)));
		KN_CORE_INFO("  Version: {0}", (const char*)(glGetString(GL_VERSION)));

		KN_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5), "Hazel requires at least OpenGL version 4.5!");
	}

	void OpenGLContext::SwapBuffers()
	{
		KN_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}