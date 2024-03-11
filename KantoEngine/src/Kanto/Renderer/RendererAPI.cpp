#include "knpch.h"
#include "Kanto/Renderer/RendererAPI.h"
#include "Kanto/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Kanto {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	Scope<RendererAPI> RendererAPI::Create()
	{
		/*switch (s_API)
		{
		case RendererAPI::API::None:    KN_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
		}

		KN_CORE_ASSERT(false, "Unknown RendererAPI!");*/
		return nullptr;
	}

}
