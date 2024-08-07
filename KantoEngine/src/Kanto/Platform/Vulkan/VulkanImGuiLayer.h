#pragma once

#include "Kanto/ImGui/ImGuiLayer.h"
//#include "Kanto/Renderer/RenderCommandBuffer.h"
#include <vulkan/vulkan_core.h>

namespace Kanto
{

	class VulkanImGuiLayer : public ImGuiLayer
	{
	public:
		VulkanImGuiLayer();
		VulkanImGuiLayer(const std::string& name);
		virtual ~VulkanImGuiLayer();

		virtual void Begin() override;
		virtual void End() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;
	private:
		//Ref<RenderCommandBuffer> m_RenderCommandBuffer;
		Ref<VkCommandBuffer> m_RenderCommandBuffer;
		float m_Time = 0.0f;
	};

}

