#pragma once
#include "knpch.h"
#include "Kanto/Core/Base.h"
#include "Application.h"

#ifdef KN_PLATFORM_WINDOWS

extern Kanto::Application* Kanto::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
	Kanto::Log::Init();

	KN_PROFILE_BEGIN_SESSION("Startup", "KantoProfile-Startup.json");
	auto app = Kanto::CreateApplication({ argc, argv });
	KN_PROFILE_END_SESSION();

	KN_PROFILE_BEGIN_SESSION("Runtime", "KantoProfile-Runtime.json");
	app->Run();
	KN_PROFILE_END_SESSION();

	KN_PROFILE_BEGIN_SESSION("Shutdown", "KantoProfile-Shutdown.json");
	delete app;
	KN_PROFILE_END_SESSION();
}

#endif
