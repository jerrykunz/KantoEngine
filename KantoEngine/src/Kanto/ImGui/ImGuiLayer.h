#pragma once

#include "Kanto/Core/Layer.h"

namespace Kanto 
{

	class ImGuiLayer : public Layer
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		void SetDarkThemeColors();
		void SetDarkThemeV2Colors();

		void AllowInputEvents(bool allowEvents);

		static ImGuiLayer* Create();
	};



}
