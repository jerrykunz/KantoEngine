#include "Kanto.h"
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
