#include "knpch.h"
#include "Kanto/Renderer/GraphicsContext.h"

#include "Kanto/Renderer/Renderer.h"
#include "Kanto/Platform/OpenGL/OpenGLContext.h"

namespace Kanto {

	Scope<GraphicsContext> GraphicsContext::Create(void* window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
		}

		KN_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
