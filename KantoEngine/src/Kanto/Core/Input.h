#pragma once

#include "Kanto/Core/KeyCodes.h"
#include "Kanto/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace Kanto {

	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}
