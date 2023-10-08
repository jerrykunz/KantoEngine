#pragma once
//#include "knpch.h"
#include <iostream>

namespace Kanto
{
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			//HZ_CORE_ASSERT(index < Count);
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string Name = "Hazel Application";
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	private:
		ApplicationSpecification m_Specification;
		static Application* s_Instance;
	public:
		static Application& Get() { return *s_Instance; }
		Application() = default;
		Application(const ApplicationSpecification& specification);
		void Run();
	};

	Application* CreateApplication(ApplicationCommandLineArgs args);
}
