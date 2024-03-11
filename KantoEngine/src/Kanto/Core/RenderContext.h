#pragma once

#include "knpch.h"

struct GLFWwindow;

namespace Kanto {

	class RendererContext
	{
	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;

		virtual void Init() = 0;

		static Ref<RendererContext> Create();
	};

}
