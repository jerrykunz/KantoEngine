#pragma once
//#include "knpch.h"
//#include "Hazel/Core/Base.h"
#include "Application.h"

//#ifdef HZ_PLATFORM_WINDOWS

extern Kanto::Application* Kanto::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	//Hazel::Log::Init();

	//HZ_PROFILE_BEGIN_SESSION("Startup", "HazelProfile-Startup.json");
	auto app = Kanto::CreateApplication({ argc, argv });
	//HZ_PROFILE_END_SESSION();

	//HZ_PROFILE_BEGIN_SESSION("Runtime", "HazelProfile-Runtime.json");
	app->Run();
	//HZ_PROFILE_END_SESSION();

	//HZ_PROFILE_BEGIN_SESSION("Shutdown", "HazelProfile-Shutdown.json");
	delete app;
	//HZ_PROFILE_END_SESSION();
}

//#endif
