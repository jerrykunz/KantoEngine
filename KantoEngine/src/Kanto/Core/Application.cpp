#include "knpch.h"
#include "Application.h"

namespace Kanto
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		//HZ_PROFILE_FUNCTION();

		//HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Set working directory here
		/*if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);*/

		/*m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);*/
	}

	void Application::Run()
	{
		while (true);
	}
}
