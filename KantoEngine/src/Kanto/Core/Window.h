#pragma once

#include "Kanto/Core/Base.h"
#include "Kanto/Core/Events/Event.h"
#include "Kanto/Renderer/GraphicsContext.h"

#include <sstream>
#include <filesystem>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Kanto 
{

	struct WindowSpecification
	{
		std::string Title = "Hazel";
		uint32_t Width = 1600;
		uint32_t Height = 900;
		bool Decorated = true;
		bool Fullscreen = false;
		bool VSync = true;
		std::filesystem::path IconPath;
	};

	//not needed
	//class VulkanSwapChain;

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const WindowSpecification& specification);
		virtual ~Window();

		virtual void Init();
		virtual void ProcessEvents();
		virtual void SwapBuffers();

		inline uint32_t GetWidth() const { return m_Data.Width; }
		inline uint32_t GetHeight() const { return m_Data.Height; }

		virtual std::pair<uint32_t, uint32_t> GetSize() const { return { m_Data.Width, m_Data.Height }; }
		virtual std::pair<float, float> GetWindowPos() const;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
		virtual void SetVSync(bool enabled);
		virtual bool IsVSync() const;
		virtual void SetResizable(bool resizable) const;

		virtual void Maximize();
		virtual void CenterWindow();

		virtual const std::string& GetTitle() const { return m_Data.Title; }
		virtual void SetTitle(const std::string& title);

		inline void* GetNativeWindow() const { return m_Window; }

		//virtual Ref<VulkanContext> GetRenderContext() { return m_RendererContext; }
		//virtual VulkanSwapChain& GetSwapChain();

	public:
		static Window* Create(const WindowSpecification& specification = WindowSpecification());

	private:
		virtual void Shutdown();
	private:
		GLFWwindow* m_Window;
		GLFWcursor* m_ImGuiMouseCursors[9] = { 0 };
		WindowSpecification m_Specification;
		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
		float m_LastFrameTime = 0.0f;

		//Ref<RendererContext> m_RendererContext;
		//Ref<VulkanContext> m_RendererContext;
		//VulkanSwapChain* m_SwapChain;
	};

}
