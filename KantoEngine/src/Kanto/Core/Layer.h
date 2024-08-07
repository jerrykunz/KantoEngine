#pragma once

//#include "Kanto/Core/Events/Event.h"
#include "Kanto/Core/Events/Event.h"
#include "Kanto/Core/TimeStep.h"

#include <string>

namespace Kanto
{

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};

}
