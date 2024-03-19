#pragma once

#include "Kanto/Core/Application.h"

extern Kanto::Application* Kanto::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace Kanto 
{

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			InitializeCore();
			Application* app = CreateApplication(argc, argv);
			KN_CORE_ASSERT(app, "Client Application is null!");
			app->Run();
			delete app;
			ShutdownCore();
		}
		return 0;
	}

}

#if HZ_DIST && HZ_PLATFORM_WINDOWS

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Hazel::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Kanto::Main(argc, argv);
}

#endif // HZ_DIST
