//Necessary includes for client app in Kanto.h
#include "Kanto.h"
//main function is in entry point
#include "Kanto/Core/EntryPoint.h"
#include <entt/include/entt/entt.hpp>

namespace Kanto
{
	class Sandbox : public Application
	{
	public:
		Sandbox(const ApplicationSpecification& specification)
			: Application(specification)
		{
			 PushLayer(new Layer());
			//PushLayer(new Sandbox2D());
		}

		~Sandbox()
		{
		}
	};

	Application* Kanto::CreateApplication(int argc, char** argv)
	{
		ApplicationSpecification spec;
		spec.Name = "Sandbox";
		spec.WorkingDirectory = "../KantoEngine";
		//spec.CommandLineArgs = args;
		entt::registry reg;
		entt::entity entity = reg.create();

		return new Sandbox(spec);
	}
}
