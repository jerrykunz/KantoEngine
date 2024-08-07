#include "knpch.h"
#include "Application.h"

#include "Kanto/Renderer/Renderer.h"
//#include "Kanto/Renderer/Framebuffer.h"
//#include "Kanto/Renderer/UI/Font.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include "Kanto/ImGui/Colors.h"

//#include "Kanto/Asset/AssetManager.h"
//#include "Kanto/Audio/AudioEngine.h"
//#include "Kanto/Audio/AudioEvents/AudioCommandRegistry.h"

#include "Input.h"

//#include "Kanto/Physics/PhysicsSystem.h"

//#include "Kanto/Platform/Vulkan/VulkanRenderer.h"
//#include "Kanto/Platform/Vulkan/VulkanAllocator.h"
#include "Kanto/Platform/Vulkan/VulkanSwapChain.h"
#include "imgui_internal.h"

#include "Kanto/Utilities/StringUtils.h"
#include "Kanto/Debug/Profiler.h"

//#include "Kanto/Editor/EditorApplicationSettings.h"

#include <filesystem>
#include <nfd.hpp>

#include "Memory.h"

#include "Kanto/Platform/Vulkan/VulkanContext.h"
#include "Base.h"
#include "Camera.h"
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Kanto/Renderer/UI/Font.h"

extern bool g_ApplicationRunning;
extern ImGuiContext* GImGui;
namespace Kanto
{

#define BIND_EVENT_FN(fn) std::bind(&Application::##fn, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	static std::thread::id s_MainThreadID;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification), /*m_RenderThread(specification.CoreThreadingPolicy),*/ m_AppSettings("App.hsettings")
	{
		s_Instance = this;
		s_MainThreadID = std::this_thread::get_id();

		m_AppSettings.Deserialize();

		//m_RenderThread.Run();

		if (!specification.WorkingDirectory.empty())
			std::filesystem::current_path(specification.WorkingDirectory);

		m_Profiler = hnew PerformanceProfiler(); 

		//Renderer::SetConfig(specification.RenderConfig);

		WindowSpecification windowSpec;
		windowSpec.Title = specification.Name;
		windowSpec.Width = specification.WindowWidth;
		windowSpec.Height = specification.WindowHeight;
		windowSpec.Decorated = specification.WindowDecorated;
		windowSpec.Fullscreen = specification.Fullscreen;
		windowSpec.VSync = specification.VSync;
		windowSpec.IconPath = specification.IconPath;
		m_Window = std::unique_ptr<Window>(Window::Create(windowSpec));
		m_Window->Init();
		m_Window->SetEventCallback([this](Event& e) { OnEvent(e); });

		// Load editor settings (will generate default settings if the file doesn't exist yet)
		//EditorApplicationSettingsSerializer::Init();

		KN_CORE_VERIFY(NFD::Init() == NFD_OKAY);

		// Init renderer and execute command queue to compile all shaders
		//Renderer::Init();
		// Render one frame (TODO: maybe make a func called Pump or something)
		//m_RenderThread.Pump();

		if (specification.StartMaximized)
			m_Window->Maximize();
		else
			m_Window->CenterWindow();
		m_Window->SetResizable(specification.Resizable);

		if (m_Specification.EnableImGui)
		{
			m_ImGuiLayer = ImGuiLayer::Create();
			PushOverlay(m_ImGuiLayer);
		}

		//PhysicsSystem::Init();
		//ScriptEngine::Init(specification.ScriptConfig);

		//MiniAudioEngine::Init();
		//Font::Init();
	}

	Application::~Application()
	{
		NFD::Quit();

		//EditorApplicationSettingsSerializer::SaveSettings();

		m_Window->SetEventCallback([](Event& e) {});

		//m_RenderThread.Terminate();

		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}

		//ScriptEngine::Shutdown();
		//Project::SetActive(nullptr);
		//Font::Shutdown();
		//MiniAudioEngine::Shutdown();

		//Renderer::Shutdown();

		delete m_Profiler;
		m_Profiler = nullptr;
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::PopOverlay(Layer* layer)
	{
		m_LayerStack.PopOverlay(layer);
		layer->OnDetach();
	}

	void Application::RenderImGui()
	{
		KN_PROFILE_FUNC();
		KN_SCOPE_PERF("Application::RenderImGui");

		m_ImGuiLayer->Begin();

		for (int i = 0; i < m_LayerStack.Size(); i++)
			m_LayerStack[i]->OnImGuiRender();
	}

	void Application::Run()
	{
		OnInit();

		//GET CONTEXT AND UPDATE DS
		Ref<VulkanContext> rendererContext = m_Window->GetRenderContext(); //Application::Get().GetWindow().GetRenderContext();
		rendererContext->CreateDescriptorSets();

		//CREATE CAMERA
		std::pair winSize = m_Window->GetSize();
		Camera camera;
		camera.type = Camera::CameraType::firstperson;
		camera.flipY = -1.0f;
		camera.setPerspective(60.0f, (float)winSize.first / (float)winSize.second, 0.1f, 512.0f);
		camera.setTranslation(glm::vec3(1.0f, 0.0f, -2.0f));
		camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		camera.movementSpeed = 3.0f;


		//LOOP INIT
		const double targetFrameRate = 60.0;
		const double frameTime = 1.0 / targetFrameRate;
		double lastFrameTime = glfwGetTime();

		Input::SetCursorMode(CursorMode::Locked);

		float prevMouseInit = 1.0f;
		float prevMouseX = 0;
		float prevMouseY = 0;


		VulkanImage circleTexture("textures/perfectcircle.png",
			rendererContext->PhysicalDevice->Device,
			*rendererContext->Device,
			rendererContext->CommandPool,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		//box2d test
		float minusValue = 1.0f;
		b2World* world = new b2World({ 0.0f, -9.8f });

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(0.0f, 10.0f);
		bodyDef.angle = 0.0f;
		b2Body* body = world->CreateBody(&bodyDef);
		body->SetFixedRotation(true);

		b2PolygonShape boxShape;
		boxShape.SetAsBox(0.5f - boxShape.m_radius * minusValue, 0.5f - boxShape.m_radius * minusValue);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 1.0f;
		fixtureDef.restitution = 0.8f;
		fixtureDef.restitutionThreshold = 1.0f;

		body->CreateFixture(&fixtureDef);

		b2BodyDef staticBodyDef;
		staticBodyDef.type = b2_staticBody;
		staticBodyDef.position.Set(0.0f, -2.0f);
		staticBodyDef.angle = 0.0f;
		b2Body* staticBody = world->CreateBody(&staticBodyDef);

		b2PolygonShape staticBoxShape;
		staticBoxShape.SetAsBox(0.5f - staticBoxShape.m_radius * minusValue, 0.5f - staticBoxShape.m_radius * minusValue); // Set the box dimensions

		b2FixtureDef staticFixtureDef;
		staticFixtureDef.shape = &staticBoxShape;
		staticFixtureDef.friction = 1.0f;
		staticFixtureDef.restitution = 0.8f;
		staticFixtureDef.restitutionThreshold = 1.0f;

		staticBody->CreateFixture(&staticFixtureDef);

		Font::Init();

		//Ref<Font> defaultFont = Font::GetDefaultFont();
		Ref<Font> defaultFont = Font::GetDefaultMonoSpacedFont();
		VulkanImage atlas = *defaultFont->GetFontAtlas().get();

		//LOOP START
		while (m_Running)
		{
			ProcessEvents();

			double currentFrameTime = glfwGetTime();
			double deltaTime = currentFrameTime - lastFrameTime;

			if (deltaTime >= frameTime)
			{
				lastFrameTime = currentFrameTime;

				camera.keys.left = Input::IsKeyDown(KeyCode::A);
				camera.keys.right = Input::IsKeyDown(KeyCode::D);
				camera.keys.forward = Input::IsKeyDown(KeyCode::W);
				camera.keys.backward = Input::IsKeyDown(KeyCode::S);
				camera.keys.up = Input::IsKeyDown(KeyCode::Space);
				camera.keys.down = Input::IsKeyDown(KeyCode::LeftControl);

				auto [mouseX, mouseY] = Input::GetMouseDelta();
				camera.rotate(glm::vec3(-mouseY * 0.01f,
										mouseX * 0.01f, 
										0.0f));			

				camera.update(deltaTime);

				auto viewProjUBO = rendererContext->GetViewProjectionUBO();
				viewProjUBO->proj = camera.matrices.perspective;
				viewProjUBO->view = camera.matrices.view;


				world->Step(deltaTime, 6, 2);

				rendererContext->BeginFrame();
				rendererContext->BeginScene();

				rendererContext->RenderLine(glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(20.0f, 0.0f, 0.0f),
					glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
					glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

				rendererContext->RenderLine(glm::vec3(20.0f, 0.0f, 0.0f),
					glm::vec3(20.0f, 20.0f, 0.0f),
					glm::vec4(1.0f, 0.0f, 0.0f, 0.01f),
					glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

				rendererContext->RenderQuadLine(glm::vec3(0.0f, 0.0f, 0.11f),
					glm::vec3(2.0f, 2.0f, 0.11f),
					0.01f,
					glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
					glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
					glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
					glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

				rendererContext->RenderQuad(glm::mat4(1.0f),
					atlas,
					1.0f,
					glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
					glm::vec2(0.0f, 0.0f),
					glm::vec2(1.0f, 1.0f));

				//rendererContext->DrawString("KIPSU", glm::vec3(0.0f, 0.0f, 0.0f), 100.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

				{
					b2Vec2 b2Position = body->GetPosition();
					float b2Angle = body->GetAngle();
					glm::vec3 position(b2Position.x, b2Position.y, 0.0f); // Box2D is 2D, so z is 0
					glm::quat rotation = glm::angleAxis(b2Angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis

					glm::mat4 transform = glm::mat4(1.0f); // Identity matrix
					transform = glm::translate(transform, position); // Apply translation
					transform *= glm::toMat4(rotation); // Apply rotation

					glm::vec3 scale(1.0f, 1.0f, 0.0f); // Box2D is 2D, so z is 0
					transform = glm::scale(transform, scale);

					glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f); // Example color (red)

					rendererContext->RenderQuad(transform, color);


					b2Vec2 sb2Position = staticBody->GetPosition();
					float sb2Angle = staticBody->GetAngle();
					glm::vec3 sposition(sb2Position.x, sb2Position.y, 0.0f); // Box2D is 2D, so z is 0
					glm::quat srotation = glm::angleAxis(sb2Angle, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around z-axis

					glm::mat4 stransform = glm::mat4(1.0f); // Identity matrix
					stransform = glm::translate(stransform, sposition); // Apply translation
					stransform *= glm::toMat4(srotation); // Apply rotation

					glm::vec3 sscale(1.0f, 1.0f, 0.0f); // Box2D is 2D, so z is 0
					stransform = glm::scale(stransform, sscale);

					glm::vec4 scolor(1.0f, 1.0f, 0.0f, 1.0f); 

					//rendererContext->RenderQuad(stransform, scolor);
					rendererContext->RenderQuad(stransform, circleTexture, 1.0f, scolor, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

					auto v1 = body->GetPosition();
					auto v2 = staticBody->GetPosition();
					int a = 3;

				}




				//test
				//RenderImGui();



				/*ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				bool showdemo = true;
				ImGui::ShowDemoWindow(&showdemo);
				ImGui::Render();*/



				rendererContext->EndScene();

				Application* app = this;
				if (m_Specification.EnableImGui /*&& false*/)
				{
					RenderImGui();
					//ImGui::Begin("Hello, world!");
					//ImGui::Text("This is some useful text.");
					//ImGui::End();
					///*bool showdemo = true;
					//ImGui::ShowDemoWindow(&showdemo);*/

					m_ImGuiLayer->End();
				}

				rendererContext->EndFrame();
				rendererContext->Present((GLFWwindow*)m_Window->GetNativeWindow());

				//old style
				//rendererContext->DrawFrame((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());



				Input::ClearReleasedKeys();
				int a = 3;
			}
		}



		//test
		/*Ref<VulkanContext> rendererContext = Application::Get().GetWindow().GetRenderContext();
		rendererContext = VulkanContext::Get();*/
		while (true)
		{
			//swap rendercommandqueues (per frame?)
			//multithread shit
			// BEGINFRAME
				//execute m_CurrentBufferIndex
				//acquire next image (fpAcquireNextImageKHR)
				//reset command pool (vkResetCommandPool, resets all command buffers) (I'm using vkResetCommandBuffer separately)
			//RuntimeLayer::OnUpdate(Timestep ts)
				//auto [width, height] = app.GetWindow().GetSize();
				//m_SceneRenderer->SetViewportSize(width, height);
				//m_RuntimeScene->SetViewportSize(width, height);
				//m_EditorCamera.SetViewportSize(width, height);
				//m_Renderer2DProj = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

				//if (m_Width != width || m_Height != height)
				//{
				//	m_Width = width;
				//	m_Height = height;
				//	m_Renderer2D->OnRecreateSwapchain(); (just recreates swapchain with all that bs from onupdate to here)

				//	// Retrieve new main command buffer
				//	m_CommandBuffer = RenderCommandBuffer::CreateFromSwapChain("RuntimeLayer");
				//}

				//OnRenderRuntime
					//Renderer2D::BeginScene (reset all quadcounts etc)
					//Renderer2D::SetTargetFramebuffer(Ref<Framebuffer> framebuffer) (set target framebuffer to all graphicspipelines)
					//Renderer2D::EndScene() (actually draw quads, texts, lines, etc)

				//same for screenspacerenderer2d (???)
			//RuntimeLayer::OnRender2D() <- what is this now, this does basically nothing? begin and end again? I guess it's debug info
			//// Render final image to swapchain

		}


		/*while (m_Running)
		{
			// Wait for render thread to finish frame
			{
				KN_PROFILE_SCOPE("Wait");
				Timer timer;

				m_RenderThread.BlockUntilRenderComplete();

				m_PerformanceTimers.MainThreadWaitTime = timer.ElapsedMillis();
			}

			static uint64_t frameCounter = 0;
			//KN_CORE_INFO("-- BEGIN FRAME {0}", frameCounter);

			ProcessEvents(); // Poll events when both threads are idle

			m_ProfilerPreviousFrameData = m_Profiler->GetPerFrameData();
			m_Profiler->Clear();

			m_RenderThread.NextFrame();

			// Start rendering previous frame
			m_RenderThread.Kick();

			if (!m_Minimized)
			{
				Timer cpuTimer;

				// On Render thread
				Renderer::Submit([&]()
					{
						m_Window->GetSwapChain().BeginFrame();
					});

				Renderer::BeginFrame();
				{
					KN_SCOPE_PERF("Application Layer::OnUpdate");
					for (Layer* layer : m_LayerStack)
						layer->OnUpdate(m_TimeStep);
				}

				Ref<Scene> activeScene = ScriptEngine::GetSceneContext();
				if (activeScene)
				{
					m_PerformanceTimers.ScriptUpdate = activeScene->GetPerformanceTimers().ScriptUpdate;
					m_PerformanceTimers.PhysicsStepTime = activeScene->GetPerformanceTimers().PhysicsStep;
				}

				// Render ImGui on render thread
				Application* app = this;
				if (m_Specification.EnableImGui)
				{
					Renderer::Submit([app]() { app->RenderImGui(); });
					Renderer::Submit([=]() { m_ImGuiLayer->End(); });
				}
				Renderer::EndFrame();

				// On Render thread
				Renderer::Submit([&]()
					{
						// m_Window->GetSwapChain().BeginFrame();
						// Renderer::WaitAndRender();
						m_Window->SwapBuffers();
					});

				m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % Renderer::GetConfig().FramesInFlight;
				m_PerformanceTimers.MainThreadWorkTime = cpuTimer.ElapsedMillis();
			}

			//ScriptEngine::InitializeRuntimeDuplicatedEntities();
			Input::ClearReleasedKeys();

			float time = GetTime();
			m_Frametime = time - m_LastFrameTime;
			m_TimeStep = glm::min<float>(m_Frametime, 0.0333f);
			m_LastFrameTime = time;

			//KN_CORE_INFO("-- END FRAME {0}", frameCounter);
			frameCounter++;

			KN_PROFILE_MARK_FRAME;
		}*/
		OnShutdown();
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnShutdown()
	{
		m_EventCallbacks.clear();
		g_ApplicationRunning = false;
	}

	void Application::ProcessEvents()
	{
		Input::TransitionPressedKeys();
		Input::TransitionPressedButtons();

		m_Window->ProcessEvents();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		// Process custom event queue
		while (m_EventQueue.size() > 0)
		{
			auto& func = m_EventQueue.front();
			func();
			m_EventQueue.pop();
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return OnWindowResize(e); });
		dispatcher.Dispatch<WindowMinimizeEvent>([this](WindowMinimizeEvent& e) { return OnWindowMinimize(e); });
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return OnWindowClose(e); });

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}

		if (event.Handled)
			return;

		// TODO(Peter): Should these callbacks be called BEFORE the layers recieve events?
		//				We may actually want that since most of these callbacks will be functions REQUIRED in order for the game
		//				to work, and if a layer has already handled the event we may end up with problems
		for (auto& eventCallback : m_EventCallbacks)
		{
			eventCallback(event);

			if (event.Handled)
				break;
		}

	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		/*const uint32_t width = e.GetWidth(), height = e.GetHeight();
		if (width == 0 || height == 0)
		{
			//m_Minimized = true;
			return false;
		}
		//m_Minimized = false;

		auto& window = m_Window;
		Renderer::Submit([&window, width, height]() mutable
			{
				window->GetSwapChain().OnResize(width, height);
			});

		return false;*/
		return false;
	}

	bool Application::OnWindowMinimize(WindowMinimizeEvent& e)
	{
		m_Minimized = e.IsMinimized();
		return false;
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		Close();
		return false; // give other things a chance to react to window close
	}

	float Application::GetTime() const
	{
		return (float)glfwGetTime();
	}

	const char* Application::GetConfigurationName()
	{
#if defined(KN_DEBUG)
		return "Debug";
#elif defined(KN_RELEASE)
		return "Release";
#elif defined(KN_DIST)
		return "Dist";
#else
#error Undefined configuration?
#endif
	}

	const char* Application::GetPlatformName()
	{
#if defined(KN_PLATFORM_WINDOWS)
		return "Windows x64";
#elif defined(KN_PLATFORM_LINUX)
		return "Linux";
#else
		return "Unknown"
#endif
	}

	std::thread::id Application::GetMainThreadID() { return s_MainThreadID; }

}
