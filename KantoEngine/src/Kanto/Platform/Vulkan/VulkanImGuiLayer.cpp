#include "knpch.h"

#include "VulkanImGuiLayer.h"

#include "imgui.h"
#include "Kanto/ImGui/ImGuizmo.h"
#include "Kanto/ImGui/ImGuiFonts.h"
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Kanto/Core/Application.h"
#include <GLFW/glfw3.h>

#include "Kanto/Editor/FontAwesome.h"

#include "Kanto/Renderer/Renderer.h"

#include "Kanto/Platform/Vulkan/VulkanContext.h"
#include "VulkanUtils.h"

namespace Kanto 
{

	static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;

	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	VulkanImGuiLayer::VulkanImGuiLayer()
	{
	}

	VulkanImGuiLayer::VulkanImGuiLayer(const std::string& name)
	{

	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{
	}

	void VulkanImGuiLayer::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Configure Fonts
		if (false)
		{
			UI::FontConfiguration robotoBold;
			robotoBold.FontName = "Bold";
			robotoBold.FilePath = "Resources/Fonts/Roboto/Roboto-Bold.ttf";
			robotoBold.Size = 18.0f;
			UI::Fonts::Add(robotoBold);

			UI::FontConfiguration robotoLarge;
			robotoLarge.FontName = "Large";
			robotoLarge.FilePath = "Resources/Fonts/Roboto/Roboto-Regular.ttf";
			robotoLarge.Size = 24.0f;
			UI::Fonts::Add(robotoLarge);

			UI::FontConfiguration robotoDefault;
			robotoDefault.FontName = "Default";
			robotoDefault.FilePath = "Resources/Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoDefault.Size = 15.0f;
			UI::Fonts::Add(robotoDefault, true);

			static const ImWchar s_FontAwesomeRanges[] = { KN_ICON_MIN, KN_ICON_MAX, 0 };
			UI::FontConfiguration fontAwesome;
			fontAwesome.FontName = "FontAwesome";
			fontAwesome.FilePath = "Resources/Fonts/FontAwesome/fontawesome-webfont.ttf";
			fontAwesome.Size = 16.0f;
			fontAwesome.GlyphRanges = s_FontAwesomeRanges;
			fontAwesome.MergeWithLast = true;
			UI::Fonts::Add(fontAwesome);

			UI::FontConfiguration robotoMedium;
			robotoMedium.FontName = "Medium";
			robotoMedium.FilePath = "Resources/Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoMedium.Size = 18.0f;
			UI::Fonts::Add(robotoMedium);

			UI::FontConfiguration robotoSmall;
			robotoSmall.FontName = "Small";
			robotoSmall.FilePath = "Resources/Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoSmall.Size = 12.0f;
			UI::Fonts::Add(robotoSmall);

			UI::FontConfiguration robotoExtraSmall;
			robotoExtraSmall.FontName = "ExtraSmall";
			robotoExtraSmall.FilePath = "Resources/Fonts/Roboto/Roboto-SemiMedium.ttf";
			robotoExtraSmall.Size = 10.0f;
			UI::Fonts::Add(robotoExtraSmall);

			UI::FontConfiguration robotoBoldTitle;
			robotoBoldTitle.FontName = "BoldTitle";
			robotoBoldTitle.FilePath = "Resources/Fonts/Roboto/Roboto-Bold.ttf";
			robotoBoldTitle.Size = 16.0f;
			UI::Fonts::Add(robotoBoldTitle);
		}
		
		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		SetDarkThemeV2Colors();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);
		
		VulkanImGuiLayer* instance = this;

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		auto vulkanContext = VulkanContext::Get();
		auto device = VulkanContext::Get()->Device->Device; //VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VkDescriptorPool descriptorPool;

		// Create Descriptor Pool
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool));

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.MSAASamples = vulkanContext->PhysicalDevice->MsaaSamples;
		init_info.Instance = vulkanContext->Instance; //VulkanContext::GetInstance();
		init_info.PhysicalDevice = vulkanContext->PhysicalDevice->Device; //VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = vulkanContext->PhysicalDevice->QueueIndices.graphicsFamily.value(); //VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
		init_info.Queue = vulkanContext->Device->GraphicsQueue; //VulkanContext::GetCurrentDevice()->GetGraphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.RenderPass = vulkanContext->RenderPass;
		VulkanSwapChain& swapChain = Application::Get().GetWindow().GetSwapChain();
		init_info.ImageCount = swapChain.SwapChainImages.size(); //swapChain.GetImageCount();
		init_info.CheckVkResultFn = Utils::VulkanCheckResult;
		//ImGui_ImplVulkan_Init(&init_info, vulkanContext->RenderPass /*swapChain.GetRenderPass()*/);
		//ImGui_ImplVulkan_Init(&init_info, vulkanContext->RenderPass /*swapChain.GetRenderPass()*/);
		//init_info.PipelineCache = g_PipelineCache;

		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info);

		// Upload Fonts
		if (true)
		{
			//new version
			//works without this too
			ImGui_ImplVulkan_CreateFontsTexture();

			//old
			// Use any command queue
			//VkCommandBuffer commandBuffer = vulkanContext->CommandBuffers[vulkanContext->CurrentFrame]; //vulkanContext->GetCurrentDevice()->GetCommandBuffer(true);
			//{
			//	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
			//	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			//	VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo));
			//}


			//ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
			////vulkanContext->GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
			//vulkanContext->FlushCommandBuffer(commandBuffer); //here is where shit goes wrong, commandbuffer gets destroyed. Fixed, no longer happens.

			//VK_CHECK_RESULT(vkDeviceWaitIdle(device));
			//ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
		
		uint32_t framesInFlight = vulkanContext->MaxFramesInFlight; //Renderer::GetConfig().FramesInFlight;
		s_ImGuiCommandBuffers.resize(framesInFlight);

		for (uint32_t i = 0; i < framesInFlight; i++)
			s_ImGuiCommandBuffers[i] = vulkanContext->CreateSecondaryCommandBuffer("ImGuiSecondaryCommandBuffer"); //VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer("ImGuiSecondaryCommandBuffer");
			

	}

	void VulkanImGuiLayer::OnDetach()
	{
		auto device = VulkanContext::Get()->Device->Device; //VulkanContext::GetCurrentDevice()->GetVulkanDevice();

		VK_CHECK_RESULT(vkDeviceWaitIdle(device));
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::Begin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	//somehow this works
	void VulkanImGuiLayer::End()
	{
		ImGui::Render();

		VulkanSwapChain& swapChain = Application::Get().GetWindow().GetSwapChain();

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		auto winSize = Application::Get().GetWindow().GetSize();

		uint32_t width = winSize.first; //swapChain.GetWidth();
		uint32_t height = winSize.second; //swapChain.GetHeight();

		uint32_t commandBufferIndex = VulkanContext::Get()->CurrentFrame; //swapChain.GetCurrentBufferIndex();

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		VkCommandBuffer drawCommandBuffer = VulkanContext::Get()->CommandBuffers[VulkanContext::Get()->CurrentFrame]; //swapChain.GetCurrentDrawCommandBuffer();
		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo));

		VkFramebuffer frameBuffer = VulkanContext::Get()->FrameBuffer->SwapChainFramebuffers[VulkanContext::Get()->ImageIndex];

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = VulkanContext::Get()->RenderPass; //swapChain.GetRenderPass();
		//renderPassBeginInfo.renderPass = VulkanContext::Get()->RenderPassImgui;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2; // Color + depth
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = frameBuffer; //swapChain.GetCurrentFramebuffer();

		vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = VulkanContext::Get()->RenderPass; //swapChain.GetRenderPass();
		//inheritanceInfo.renderPass = VulkanContext::Get()->RenderPassImgui; //swapChain.GetRenderPass();
		inheritanceInfo.framebuffer = frameBuffer; //swapChain.GetCurrentFramebuffer();

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

		VK_CHECK_RESULT(vkBeginCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex], &cmdBufInfo));

		//original, causes errors
		/*VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);*/

		//alternative without errors
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;  // Set y coordinate to 0
		viewport.height = (float)height;  // Set height to positive value
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(s_ImGuiCommandBuffers[commandBufferIndex], 0, 1, &scissor);

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_ImGuiCommandBuffers[commandBufferIndex]);


		VK_CHECK_RESULT(vkEndCommandBuffer(s_ImGuiCommandBuffers[commandBufferIndex]));

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(s_ImGuiCommandBuffers[commandBufferIndex]);

		vkCmdExecuteCommands(drawCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());

		vkCmdEndRenderPass(drawCommandBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(drawCommandBuffer));

		// Update and Render additional Platform Windows
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGuiLayer::OnImGuiRender()
	{
		/*ImGui::Begin("Hello, world!");
		ImGui::Text("This is some useful text.");
		ImGui::End();*/

		//crashes everything
		bool showdemo = true;
		ImGui::ShowDemoWindow(&showdemo);
	}

}

