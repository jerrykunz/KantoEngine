//Necessary includes for client app in Kanto.h
#include "Kanto.h"
//main function is in entry point
#include "Kanto/Core/EntryPoint.h"

namespace Kanto
{
	class Sandbox : public Application
	{
	public:
		Sandbox(const ApplicationSpecification& specification)
			: Application(specification)
		{
			// PushLayer(new ExampleLayer());
			//PushLayer(new Sandbox2D());
		}

		~Sandbox()
		{
		}
	};

	Application* Kanto::CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "Sandbox";
		spec.WorkingDirectory = "../KantoEngine";
		spec.CommandLineArgs = args;

		return new Sandbox(spec);
	}
}
