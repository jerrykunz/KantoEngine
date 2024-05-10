#include "knpch.h"
#include "VulkanContext.h"
#include "Kanto/Core/Application.h"
#include "imgui.h"
#include "VulkanUtils.h"

#include "imgui.h"
#include "Kanto/ImGui/ImGuizmo.h"
#include "Kanto/ImGui/ImGuiFonts.h"
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif
#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan_with_textures.h"



namespace Kanto
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	VulkanContext::VulkanContext(GLFWwindow* window,
								 const std::string& applicationName,
								 const std::string& engineName,
								 bool vsync)
	{
		_enableValidationLayers = true;
		MaxFramesInFlight = 2;

		_textureSlots.resize(_maxTextures);
		_textureIndex = 0;

		InitializeInstance(applicationName, engineName);
		InitializeDebugMessenger();
		CreateSurface(window);

		//test
		Utils::VulkanLoadDebugUtilsExtensions(Instance);

		PhysicalDevice = new VulkanPhysicalDevice(Instance,
			Surface);

		Device = new VulkanDevice(_enableValidationLayers,
			_validationLayers,
			*PhysicalDevice);

		VulkanSwapChainSupportDetails swapChainDetails = PhysicalDevice->QuerySwapChainSupport(Surface);
		SwapChain = new VulkanSwapChain(*window,
			Device->Device,
			Surface,
			swapChainDetails,
			PhysicalDevice->QueueIndices,
			vsync);

		CreateRenderPass(PhysicalDevice,
			Device,
			SwapChain->SwapChainImageFormat);

		CreateImguiRenderPass(PhysicalDevice,
			Device,
			SwapChain->SwapChainImageFormat);

		ViewProjectionUniformBuffer = new VulkanUniformBuffer(PhysicalDevice->Device,
			Device->Device,
			sizeof(ViewProjectionUBO),
			MaxFramesInFlight,
			0,
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			VK_SHADER_STAGE_VERTEX_BIT);
		UniformBuffers.push_back(ViewProjectionUniformBuffer);

		InstanceDataUniformBuffer = new VulkanUniformBuffer(PhysicalDevice->Device,
			Device->Device,
			sizeof(InstanceDataUBO),
			MaxFramesInFlight,
			1,
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			nullptr,
			VK_SHADER_STAGE_VERTEX_BIT);
		UniformBuffers.push_back(InstanceDataUniformBuffer);

		CreateDescriptorSetLayout();

		_quadPipeline = new VulkanPipeline(Device->Device,
			RenderPass,
			PhysicalDevice->MsaaSamples,
			_descriptorSetLayout,
			"shaders/quad2dvert.spv",
			"shaders/quad2dfrag.spv",
			QuadVertex::getBindingDescription(),
			QuadVertex::getAttributeDescriptions2(),
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			VK_POLYGON_MODE_FILL);

		_linePipeline = new VulkanPipeline(Device->Device,
			RenderPass,
			PhysicalDevice->MsaaSamples,
			_descriptorSetLayout,
			"shaders/line2dvert.spv",
			"shaders/line2dfrag.spv",
			LineVertex::getBindingDescription(),
			LineVertex::getAttributeDescriptions(),
			VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
			VK_POLYGON_MODE_FILL);


		CreateCommandPool();

		FrameBuffer = new VulkanFrameBuffer(*PhysicalDevice,
			Device->Device,
			RenderPass,
			SwapChain->SwapChainImageViews,
			SwapChain->SwapChainExtent,
			SwapChain->SwapChainImageFormat);



		CreateDescriptorPool();
		CreateCommandBuffers();
		CreateSyncObjects();
		InitQuadRendering();
		InitLineRendering();
	}

	VulkanContext::~VulkanContext()
	{

	}

	void VulkanContext::InitQuadRendering()
	{
		uint32_t white = 0xffffffff;
		WhiteTexture = new VulkanImage(&white, PhysicalDevice->Device, *Device, CommandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		_textureSlots[_textureIndex] = WhiteTexture;
		_textureIndex++; //1 after this

		_maxQuadVertices = 40000;
		_maxQuadIndices = 60000;

		QuadVertexCount = 0;
		QuadIndexCount = 0;

		QuadVertexBuffer.resize(MaxFramesInFlight);
		QuadVertices.resize(MaxFramesInFlight);

		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			QuadVertexBuffer[i] = VulkanVertexBuffer();
			QuadVertices[i] = new QuadVertex[_maxQuadVertices];
		}

		_quadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		_quadVertexPositions[1] = { -0.5f,  0.5f, 0.0f, 1.0f };
		_quadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		_quadVertexPositions[3] = { 0.5f, -0.5f, 0.0f, 1.0f };



		//Load indices, always the same so no need for per frame indices
		QuadIndexBuffer = VulkanIndexBuffer();
		//uint32_t* quadIndices = new uint32_t[_maxQuadIndices];
		std::vector<uint32_t> quadIndices(_maxQuadIndices);
		uint32_t offset = 0;
		for (uint32_t i = 0; i < _maxQuadIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}
		QuadIndexBuffer.LoadIndices(quadIndices,
			PhysicalDevice->Device,
			Device->Device,
			Device->GraphicsQueue,
			CommandPool);
	}

	void VulkanContext::InitLineRendering()
	{
		_maxLineVertices = 20000;
		_maxLineIndices = 20000;

		LineVertexCount = 0;
		LineIndexCount = 0;

		LineVertexBuffer.resize(MaxFramesInFlight);
		LineVertices.resize(MaxFramesInFlight);

		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			LineVertexBuffer[i] = VulkanVertexBuffer();
			LineVertices[i] = new LineVertex[_maxLineVertices];
		}



		LineIndexBuffer = VulkanIndexBuffer();
		std::vector<uint32_t> lineIndices(_maxLineIndices);

		for (int i = 0; i < _maxLineIndices; i++)
		{
			lineIndices[i] = i;
		}

		LineIndexBuffer.LoadIndices(lineIndices,
			PhysicalDevice->Device,
			Device->Device,
			Device->GraphicsQueue,
			CommandPool);
	}

	void VulkanContext::BeginFrame()
	{
		vkWaitForFences(Device->Device, 1, &_inFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

		//uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(Device->Device, SwapChain->SwapChain, UINT64_MAX, _imageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(Device->Device, 1, &_inFlightFences[CurrentFrame]);
		
		vkResetCommandBuffer(CommandBuffers[CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
	}

	void VulkanContext::BeginScene()
	{
		QuadVertexCount = 0;
		QuadIndexCount = 0;

		LineVertexCount = 0;
		LineIndexCount = 0;
	}

	void VulkanContext::EndScene()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(CommandBuffers[CurrentFrame], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = RenderPass;
		renderPassInfo.framebuffer = FrameBuffer->SwapChainFramebuffers[ImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SwapChain->SwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(CommandBuffers[CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//we can do this after viewport and scissor
		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)SwapChain->SwapChainExtent.width;
		viewport.height = (float)SwapChain->SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(CommandBuffers[CurrentFrame], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SwapChain->SwapChainExtent;
		vkCmdSetScissor(CommandBuffers[CurrentFrame], 0, 1, &scissor);

		//2D quad rendering
		if (QuadVertexCount > 0)
		{
			vkCmdBindPipeline(CommandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->Pipeline  /*_GraphicsPipeline2DQuad*/);

			//moved here, still causes errors
			UpdateTextureDescriptorSets();

			vkCmdBindDescriptorSets(CommandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->PipelineLayout /*_pipelineLayout2DQuad*/, 0, 1, &_descriptorSets[CurrentFrame], 0, nullptr);

			QuadVertexBuffer[CurrentFrame].LoadVertices(QuadVertices[CurrentFrame],
				QuadVertexCount,
				PhysicalDevice->Device,
				Device->Device,
				Device->GraphicsQueue,
				CommandPool);

			VkBuffer vertexBuffers[] = { QuadVertexBuffer[CurrentFrame].VertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(CommandBuffers[CurrentFrame], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(CommandBuffers[CurrentFrame], QuadIndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(CommandBuffers[CurrentFrame],
				static_cast<uint32_t>(QuadIndexCount * sizeof(uint32_t)),
				1,
				0,
				0,
				0);

			QuadVertexCount = 0;
			QuadIndexCount = 0;
		}
		//END 2D quad rendering END

		//2d line rendering
		if (LineVertexCount > 0)
		{
			vkCmdBindPipeline(CommandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _linePipeline->Pipeline);

			vkCmdBindDescriptorSets(CommandBuffers[CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, _linePipeline->PipelineLayout, 0, 1, &_descriptorSets[CurrentFrame], 0, nullptr);

			LineVertexBuffer[CurrentFrame].LoadVertices(LineVertices[CurrentFrame],
				LineVertexCount,
				PhysicalDevice->Device,
				Device->Device,
				Device->GraphicsQueue,
				CommandPool);

			VkBuffer vertexBuffers[] = { LineVertexBuffer[CurrentFrame].VertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(CommandBuffers[CurrentFrame], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(CommandBuffers[CurrentFrame], LineIndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(CommandBuffers[CurrentFrame],
				static_cast<uint32_t>(LineIndexCount * sizeof(uint32_t)),
				1,
				0,
				0,
				0);

			LineVertexCount = 0;
			LineIndexCount = 0;
		}
		//END 2D line rendering END

		 //IMGUI (test)
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), CommandBuffers[CurrentFrame]);


			ImGuiIO& io = ImGui::GetIO(); (void)io;
			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		}


		vkCmdEndRenderPass(CommandBuffers[CurrentFrame]);

		if (vkEndCommandBuffer(CommandBuffers[CurrentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];

		VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(Device->GraphicsQueue, 1, &submitInfo, _inFlightFences[CurrentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		//TEST
		VK_CHECK_RESULT(vkWaitForFences(Device->Device, 1, &_inFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX));

		//these two cause issues most likely
		//VK_CHECK_RESULT(vkResetFences(Device->Device, 1, &_inFlightFences[CurrentFrame]));

		// Reset the command buffer
		//VK_CHECK_RESULT(vkResetCommandBuffer(CommandBuffers[CurrentFrame], 0));
	}

	void VulkanContext::EndFrame()
	{

	}

	void VulkanContext::Present(GLFWwindow* window)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;

		VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[CurrentFrame] };
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { SwapChain->SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &ImageIndex;

		VkResult result = vkQueuePresentKHR(Device->PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FrameBufferResized)
		{
			FrameBufferResized = false;
			RecreateSwapChain(window);
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;

		//Reset Quad rendering textures
		_textureIndex = 1;
	}

	void VulkanContext::DrawFrame(GLFWwindow* window)
	{
		vkWaitForFences(Device->Device, 1, &_inFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

		//uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(Device->Device, SwapChain->SwapChain, UINT64_MAX, _imageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain(window);
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		vkResetFences(Device->Device, 1, &_inFlightFences[CurrentFrame]);

		vkResetCommandBuffer(CommandBuffers[CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);

		RecordCommandBuffer(CommandBuffers[CurrentFrame]);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffers[CurrentFrame];

		VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(Device->GraphicsQueue, 1, &submitInfo, _inFlightFences[CurrentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { SwapChain->SwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &ImageIndex;

		result = vkQueuePresentKHR(Device->PresentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FrameBufferResized)
		{
			FrameBufferResized = false;
			RecreateSwapChain(window);
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}

		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;

		//Reset Quad rendering textures
		_textureIndex = 1;
	}

	void VulkanContext::CleanUp()
	{
		//destroy ImGui
		VK_CHECK_RESULT(vkDeviceWaitIdle(Device->Device));
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();


		delete FrameBuffer;
		FrameBuffer = nullptr;

		delete SwapChain;
		SwapChain = nullptr;

		/*vkDestroyPipeline(Device->Device, _GraphicsPipeline2DQuad, nullptr);
		vkDestroyPipelineLayout(Device->Device, _pipelineLayout2DQuad, nullptr);*/
		_quadPipeline->Dispose(Device->Device);

		vkDestroyRenderPass(Device->Device, RenderPass, nullptr);

		delete ViewProjectionUniformBuffer;
		ViewProjectionUniformBuffer = nullptr;

		delete InstanceDataUniformBuffer;
		InstanceDataUniformBuffer = nullptr;

		vkDestroyDescriptorPool(Device->Device, _descriptorPool, nullptr);

		for (auto img : Images)
		{
			delete img;
		}
		Images.clear();

		vkDestroyDescriptorSetLayout(Device->Device, _descriptorSetLayout, nullptr);

		/*delete IndexBuffer;
		IndexBuffer = nullptr;

		delete VertexBuffer;
		VertexBuffer = nullptr;*/

		for (size_t i = 0; i < MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(Device->Device, _renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(Device->Device, _imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(Device->Device, _inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(Device->Device, CommandPool, nullptr);

		delete Device;
		Device = nullptr;

		if (_enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(nullptr);
		}

		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		vkDestroyInstance(Instance, nullptr);
	}

	//void VulkanContext::RecordImGuiCommandBuffer(/*VkCommandBuffer commandBuffer, uint32_t imageIndex*/)
	//{
	//	ImGui_ImplVulkan_NewFrame();
	//	ImGui_ImplGlfw_NewFrame();
	//	ImGui::NewFrame();
	//	ImGuizmo::BeginFrame();

	//	ImGui::Render();

	//	VulkanSwapChain& swapChain = Application::Get().GetWindow().GetSwapChain();

	//	VkClearValue clearValues[2];
	//	clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
	//	clearValues[1].depthStencil = { 1.0f, 0 };

	//	VkCommandBufferBeginInfo drawCmdBufInfo = {};
	//	drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//	drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	//	drawCmdBufInfo.pNext = nullptr;

	//	//VkCommandBuffer drawCommandBuffer = swapChain.GetCurrentDrawCommandBuffer();
	//	VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBuffers[CurrentFrame]   /*commandBuffer*/, &drawCmdBufInfo));

	//	VkRenderPassBeginInfo renderPassBeginInfo = {};
	//	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//	renderPassBeginInfo.pNext = nullptr;
	//	renderPassBeginInfo.renderPass = _imguiRenderPass;
	//	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	//	renderPassBeginInfo.renderArea.extent = SwapChain->SwapChainExtent;
	//	renderPassBeginInfo.clearValueCount = 2; // Color + depth
	//	renderPassBeginInfo.pClearValues = clearValues;
	//	renderPassBeginInfo.framebuffer = FrameBuffer->SwapChainFramebuffers[ImageIndex]; //swapChain.GetCurrentFramebuffer();

	//	vkCmdBeginRenderPass(CommandBuffers[CurrentFrame] /*commandBuffer*/, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	//	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	//	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	//	inheritanceInfo.renderPass = _imguiRenderPass;
	//	inheritanceInfo.framebuffer = FrameBuffer->SwapChainFramebuffers[ImageIndex];

	//	VkCommandBufferBeginInfo cmdBufInfo = {};
	//	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	//	cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	//	cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

	//	VK_CHECK_RESULT(vkBeginCommandBuffer(_imguiCommandBuffers[ImageIndex], &cmdBufInfo));

	//	VkViewport viewport = {};
	//	viewport.x = 0.0f;
	//	viewport.y = (float)SwapChain->SwapChainExtent.height;
	//	viewport.height = -(float)SwapChain->SwapChainExtent.height;
	//	viewport.width = (float)SwapChain->SwapChainExtent.width;
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;
	//	vkCmdSetViewport(_imguiCommandBuffers[ImageIndex], 0, 1, &viewport);

	//	VkRect2D scissor = {};
	//	scissor.extent.width = (float)SwapChain->SwapChainExtent.width;
	//	scissor.extent.height = (float)SwapChain->SwapChainExtent.height;
	//	scissor.offset.x = 0;
	//	scissor.offset.y = 0;
	//	vkCmdSetScissor(_imguiCommandBuffers[ImageIndex], 0, 1, &scissor);

	//	ImDrawData* main_draw_data = ImGui::GetDrawData();
	//	ImGui_ImplVulkan_RenderDrawData(main_draw_data, _imguiCommandBuffers[ImageIndex]);

	//	VK_CHECK_RESULT(vkEndCommandBuffer(_imguiCommandBuffers[ImageIndex]));

	//	std::vector<VkCommandBuffer> commandBuffers;
	//	commandBuffers.push_back(_imguiCommandBuffers[ImageIndex]);

	//	vkCmdExecuteCommands(CommandBuffers[CurrentFrame] /*commandBuffer*/, uint32_t(commandBuffers.size()), commandBuffers.data());

	//	vkCmdEndRenderPass(CommandBuffers[CurrentFrame]   /*commandBuffer*/);

	//	VK_CHECK_RESULT(vkEndCommandBuffer(CommandBuffers[CurrentFrame]   /*commandBuffer*/));

	//	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//	// Update and Render additional Platform Windows
	//	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//	{
	//		ImGui::UpdatePlatformWindows();
	//		ImGui::RenderPlatformWindowsDefault();
	//	}
	//}

	void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = RenderPass;
		renderPassInfo.framebuffer = FrameBuffer->SwapChainFramebuffers[ImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = SwapChain->SwapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		//we can do this after viewport and scissor
		//vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)SwapChain->SwapChainExtent.width;
		viewport.height = (float)SwapChain->SwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = SwapChain->SwapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		//2D quad rendering
		if (QuadVertexCount > 0)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->Pipeline  /*_GraphicsPipeline2DQuad*/);

			//moved here, still causes errors
			UpdateTextureDescriptorSets();

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->PipelineLayout /*_pipelineLayout2DQuad*/, 0, 1, &_descriptorSets[CurrentFrame], 0, nullptr);

			QuadVertexBuffer[CurrentFrame].LoadVertices(QuadVertices[CurrentFrame],
				QuadVertexCount,
				PhysicalDevice->Device,
				Device->Device,
				Device->GraphicsQueue,
				CommandPool);

			VkBuffer vertexBuffers[] = { QuadVertexBuffer[CurrentFrame].VertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, QuadIndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(QuadIndexCount * sizeof(uint32_t)),
				1,
				0,
				0,
				0);

			QuadVertexCount = 0;
			QuadIndexCount = 0;
		}
		//END 2D quad rendering END

		//2d line rendering
		if (LineVertexCount > 0)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _linePipeline->Pipeline);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _linePipeline->PipelineLayout, 0, 1, &_descriptorSets[CurrentFrame], 0, nullptr);

			LineVertexBuffer[CurrentFrame].LoadVertices(LineVertices[CurrentFrame],
				LineVertexCount,
				PhysicalDevice->Device,
				Device->Device,
				Device->GraphicsQueue,
				CommandPool);

			VkBuffer vertexBuffers[] = { LineVertexBuffer[CurrentFrame].VertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffer, LineIndexBuffer.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(LineIndexCount * sizeof(uint32_t)),
				1,
				0,
				0,
				0);

			LineVertexCount = 0;
			LineIndexCount = 0;
		}
		//END 2D line rendering END

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void VulkanContext::CreateImGuiRenderPass(VulkanPhysicalDevice* physicalDevice,
											  VulkanDevice* device,
											  VkFormat& swapChainImageFormat)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = physicalDevice->MsaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat(physicalDevice->Device);
		depthAttachment.samples = physicalDevice->MsaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device->Device, &renderPassInfo, nullptr, &_imguiRenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanContext::CreateImGui()
	{
		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		auto vulkanContext = VulkanContext::Get();
		auto device = Device->Device;

		//VkDescriptorPool descriptorPool;

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
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &pool_info, nullptr, &_imguiDescriptorPool));

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};

		//was missing
		init_info.MSAASamples = PhysicalDevice->MsaaSamples;

		init_info.Instance = Instance;
		init_info.PhysicalDevice = PhysicalDevice->Device; //VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetVulkanPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = (uint32_t)PhysicalDevice->QueueIndices.graphicsFamily.value_or(-1); //VulkanContext::GetCurrentDevice()->GetPhysicalDevice()->GetQueueFamilyIndices().Graphics;
		init_info.Queue = Device->GraphicsQueue;
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = _imguiDescriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		VulkanSwapChain& swapChain = Application::Get().GetWindow().GetSwapChain();
		init_info.ImageCount = swapChain.SwapChainImages.size(); //.GetImageCount();
		init_info.CheckVkResultFn = Utils::VulkanCheckResult;

		CreateImGuiRenderPass(PhysicalDevice, Device, SwapChain->SwapChainImageFormat);
		ImGui_ImplVulkan_Init(&init_info, _imguiRenderPass);

		// Upload Fonts
		{
			// Use any command queue

			//VkCommandBuffer commandBuffer = vulkanContext->GetCurrentDevice()->GetCommandBuffer(true);
			VkCommandBuffer tempBuff;
			CreateCommandBuffer(tempBuff);
			ImGui_ImplVulkan_CreateFontsTexture(tempBuff);
			//vkResetCommandBuffer(_imguiCommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
			VK_CHECK_RESULT(vkEndCommandBuffer(tempBuff));
			vkFreeCommandBuffers(device, CommandPool, 1, &tempBuff);

			//vulkanContext->GetCurrentDevice()->FlushCommandBuffer(_imguiCommandBuffer);


			VK_CHECK_RESULT(vkDeviceWaitIdle(device));
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}

		_imguiCommandBuffers.resize(MaxFramesInFlight);
		for (uint32_t i = 0; i < MaxFramesInFlight; i++)
			CreateCommandBuffer(_imguiCommandBuffers[i]);


		/*uint32_t framesInFlight = Renderer::GetConfig().FramesInFlight;
		s_ImGuiCommandBuffers.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
			s_ImGuiCommandBuffers[i] = VulkanContext::GetCurrentDevice()->CreateSecondaryCommandBuffer("ImGuiSecondaryCoommandBuffer");*/
	}

	void VulkanContext::FlushCommandBuffer(VkCommandBuffer commandBuffer/*, VkQueue queue*/)
	{
		auto vulkanDevice = Device->Device;
		auto queue = Device->GraphicsQueue;

		const uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000;

		KN_CORE_ASSERT(commandBuffer != VK_NULL_HANDLE);

		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Create fence to ensure that the command buffer has finished executing
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = 0;
		VkFence fence;
		VK_CHECK_RESULT(vkCreateFence(vulkanDevice, &fenceCreateInfo, nullptr, &fence));

		{
			static std::mutex submissionLock;
			std::scoped_lock<std::mutex> lock(submissionLock);

			// Submit to the queue
			VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		}
		// Wait for the fence to signal that command buffer has finished executing
		VK_CHECK_RESULT(vkWaitForFences(vulkanDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

		vkDestroyFence(vulkanDevice, fence, nullptr);
		//vkFreeCommandBuffers(vulkanDevice, CommandPool, 1, &commandBuffer); //not removing the buffer, J addition
	}

	void VulkanContext::InitializeInstance(const std::string& applicationName, const std::string& engineName)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = applicationName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = engineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (_enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
			createInfo.ppEnabledLayerNames = _validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &Instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}

	void VulkanContext::CreateSyncObjects()
	{
		_imageAvailableSemaphores.resize(MaxFramesInFlight);
		_renderFinishedSemaphores.resize(MaxFramesInFlight);
		_inFlightFences.resize(MaxFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MaxFramesInFlight; i++)
		{
			if (vkCreateSemaphore(Device->Device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(Device->Device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(Device->Device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	//should keep glfwindow out of this class maybe
	void VulkanContext::RecreateSwapChain(GLFWwindow* window)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(Device->Device);

		bool vsync = true;
		delete FrameBuffer;
		delete SwapChain;

		VulkanSwapChainSupportDetails swapChainDetails = PhysicalDevice->QuerySwapChainSupport(Surface);
		SwapChain = new VulkanSwapChain(*window,
			Device->Device,
			Surface,
			swapChainDetails,
			PhysicalDevice->QueueIndices,
			vsync);


		FrameBuffer = new VulkanFrameBuffer(*PhysicalDevice,
			Device->Device,
			RenderPass,
			SwapChain->SwapChainImageViews,
			SwapChain->SwapChainExtent,
			SwapChain->SwapChainImageFormat);
	}

	void VulkanContext::CreateDescriptorPool()
	{
		size_t sz = UniformBuffers.size();

		std::vector<VkDescriptorPoolSize> poolSizes{};
		poolSizes.resize(sz + 1);

		for (int i = 0; i < sz; i++)
		{
			poolSizes[i].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[i].descriptorCount = static_cast<uint32_t>(MaxFramesInFlight);
		}

		poolSizes[sz] = VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			//Added Maxtextures here, previously just _maxFramesInFlight, so 1 texture slot per frame
			.descriptorCount = static_cast<uint32_t>(MaxFramesInFlight * _maxTextures)
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(MaxFramesInFlight);

		if (vkCreateDescriptorPool(Device->Device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void VulkanContext::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MaxFramesInFlight, _descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MaxFramesInFlight);
		allocInfo.pSetLayouts = layouts.data();

		_descriptorSets.resize(MaxFramesInFlight);
		if (vkAllocateDescriptorSets(Device->Device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < MaxFramesInFlight; i++)
		{
			size_t ubsSz = UniformBuffers.size();
			std::vector<VkDescriptorBufferInfo> uniformBuffers(ubsSz);
			for (int j = 0; j < ubsSz; j++)
			{
				uniformBuffers[j] = VkDescriptorBufferInfo
				{
					.buffer = UniformBuffers[j]->UniformBuffers[i],
					.offset = 0,
					.range = UniformBuffers[j]->Size
				};
			}

			//size_t imgsSz = Images.size();
			size_t imgsSz = _textureIndex;

			std::vector<VkDescriptorImageInfo> imageDescriptors(_maxTextures);
			for (int j = 0; j < imgsSz; j++)
			{
				imageDescriptors[j] = _textureSlots[j]->Descriptor;
			}

			if (imgsSz < _maxTextures)
			{
				for (int j = imgsSz; j < _maxTextures; j++)
				{
					imageDescriptors[j] = _textureSlots[0]->Descriptor; //use this as a test default
				}
			}

			//here we create the uniform buffer which shaders use
			//int sz = Images.size() + 1;
			//int sz = ubsSz + imgsSz;
			int sz = ubsSz + 1; //always 1 because it holds all the textures

			std::vector<VkWriteDescriptorSet> descriptorWrites(sz);

			for (int j = 0; j < ubsSz; j++)
			{
				descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[j].dstSet = _descriptorSets[i];
				descriptorWrites[j].dstBinding = UniformBuffers[j]->DescriptorSetLayout.binding;  //0;
				descriptorWrites[j].dstArrayElement = 0;
				descriptorWrites[j].descriptorType = UniformBuffers[j]->DescriptorSetLayout.descriptorType; //VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[j].descriptorCount = UniformBuffers[j]->DescriptorSetLayout.descriptorCount; //1;
				descriptorWrites[j].pBufferInfo = &uniformBuffers[j]; //& viewProjectionBufferInfo;
			}

			//texture sampler(s) for shader use
			for (int j = ubsSz, k = 0; j < sz; j++, k++)
			{
				descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[j].dstSet = _descriptorSets[i];
				descriptorWrites[j].dstBinding = ubsSz; //1; //next available binding after ubs
				descriptorWrites[j].dstArrayElement = 0;
				descriptorWrites[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[j].descriptorCount = _maxTextures; //1;
				//descriptorWrites[j].pImageInfo = &imageDescriptors[k]; //&imageInfo;
				descriptorWrites[j].pImageInfo = imageDescriptors.data();
			}


			vkUpdateDescriptorSets(Device->Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void VulkanContext::UpdateTextureDescriptorSets()
	{
		std::vector<VkDescriptorImageInfo> imageDescriptors(_maxTextures);
		for (int j = 0; j < _textureIndex; j++)
		{
			imageDescriptors[j] = _textureSlots[j]->Descriptor;
		}

		if (_textureIndex < _maxTextures)
		{
			for (int j = _textureIndex; j < _maxTextures; j++)
			{
				imageDescriptors[j] = _textureSlots[0]->Descriptor; //use this as a test default
			}
		}

		std::vector<VkWriteDescriptorSet> descriptorWrites(1);
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = _descriptorSets[CurrentFrame]; //Only updates the currentframe, otherwise errors accessing data that's being rendered
		descriptorWrites[0].dstBinding = 2; //next available binding after ubs
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = _maxTextures;
		descriptorWrites[0].pImageInfo = imageDescriptors.data();

		vkUpdateDescriptorSets(Device->Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void VulkanContext::RenderQuad(const glm::mat4& transform, const glm::vec4& color)
	{
		//constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const float tilingFactor = 1.0f;

		QuadVertex* testPtr = nullptr;
		QuadVertex* testPtr2 = nullptr;
		for (size_t i = 0; i < 4; i++)
		{
			size_t index = QuadVertexCount + i;
			QuadVertices[CurrentFrame][index].pos = transform * _quadVertexPositions[i];
			QuadVertices[CurrentFrame][index].color = color;
			QuadVertices[CurrentFrame][index].texCoord = textureCoords[i];
			QuadVertices[CurrentFrame][index].texIndex = textureIndex;
			QuadVertices[CurrentFrame][index].tilingFactor = tilingFactor;
			testPtr = &QuadVertices[CurrentFrame][index];
		}
		testPtr2 = testPtr--;


		QuadVertexCount += 4;
		QuadIndexCount += 6;
	}

	void VulkanContext::RenderQuad(const glm::mat4& transform, VulkanImage& texture, float tilingFactor, const glm::vec4& tintColor, glm::vec2 uv0, glm::vec2 uv1)
	{
		constexpr size_t quadVertexCount = 4;
		glm::vec2 textureCoords[] = { uv0, { uv1.x, uv0.y }, uv1, { uv0.x, uv1.y } };

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < _textureIndex; i++)
		{
			if (_textureSlots[i] == &texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)_textureIndex;
			_textureSlots[_textureIndex] = &texture;
			_textureIndex++;
		}

		for (size_t i = 0; i < 4; i++)
		{
			size_t index = QuadVertexCount + i;
			QuadVertices[CurrentFrame][index].pos = transform * _quadVertexPositions[i];
			QuadVertices[CurrentFrame][index].color = tintColor;
			QuadVertices[CurrentFrame][index].texCoord = textureCoords[i];
			QuadVertices[CurrentFrame][index].texIndex = textureIndex;
			QuadVertices[CurrentFrame][index].tilingFactor = tilingFactor;
		}

		QuadVertexCount += 4;
		QuadIndexCount += 6;
	}

	void VulkanContext::RenderLine(const glm::vec3 p1, const glm::vec3 p2, const glm::vec4& color1, const glm::vec4& color2)
	{
		LineVertices[CurrentFrame][LineVertexCount].pos = p1;
		LineVertices[CurrentFrame][LineVertexCount++].color = color1;

		LineVertices[CurrentFrame][LineVertexCount].pos = p2;
		LineVertices[CurrentFrame][LineVertexCount++].color = color2;

		LineIndexCount += 2;
	}

	void VulkanContext::RenderQuadLine(glm::vec3 start, glm::vec3 end, float width, glm::vec4 startColor1, glm::vec4 startColor2, glm::vec4 endColor1, glm::vec4 endColor2)
	{

		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const float tilingFactor = 1.0f;

		glm::vec3 direction = glm::normalize(end - start);
		float length = glm::length(end - start);

		glm::vec3 perpendicular = glm::vec3(-direction.y, direction.x, direction.z) * (width * 0.5f);
		//glm::vec3 perpendicular = glm::vec3(-direction.y, direction.x, 0.0f) * (width * 0.5f);

		QuadVertices[CurrentFrame][QuadVertexCount].pos = start + perpendicular;
		QuadVertices[CurrentFrame][QuadVertexCount].color = startColor1;
		QuadVertices[CurrentFrame][QuadVertexCount].texCoord = textureCoords[0];
		QuadVertices[CurrentFrame][QuadVertexCount].texIndex = textureIndex;
		QuadVertices[CurrentFrame][QuadVertexCount].tilingFactor = tilingFactor;

		QuadVertices[CurrentFrame][QuadVertexCount + 1].pos = end + perpendicular;
		QuadVertices[CurrentFrame][QuadVertexCount + 1].color = endColor1;
		QuadVertices[CurrentFrame][QuadVertexCount + 1].texCoord = textureCoords[2];
		QuadVertices[CurrentFrame][QuadVertexCount + 1].texIndex = textureIndex;
		QuadVertices[CurrentFrame][QuadVertexCount + 1].tilingFactor = tilingFactor;

		QuadVertices[CurrentFrame][QuadVertexCount + 2].pos = end - perpendicular;
		QuadVertices[CurrentFrame][QuadVertexCount + 2].color = endColor2;
		QuadVertices[CurrentFrame][QuadVertexCount + 2].texCoord = textureCoords[3];
		QuadVertices[CurrentFrame][QuadVertexCount + 2].texIndex = textureIndex;
		QuadVertices[CurrentFrame][QuadVertexCount + 2].tilingFactor = tilingFactor;

		QuadVertices[CurrentFrame][QuadVertexCount + 3].pos = start - perpendicular;
		QuadVertices[CurrentFrame][QuadVertexCount + 3].color = startColor2;
		QuadVertices[CurrentFrame][QuadVertexCount + 3].texCoord = textureCoords[1];
		QuadVertices[CurrentFrame][QuadVertexCount + 3].texIndex = textureIndex;
		QuadVertices[CurrentFrame][QuadVertexCount + 3].tilingFactor = tilingFactor;


		QuadVertexCount += 4;
		QuadIndexCount += 6;
	}

	

	std::vector<const char*> VulkanContext::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (_enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}


	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : _validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	//DEBUG MESSENGER

	void VulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void VulkanContext::InitializeDebugMessenger()
	{
		if (!_enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	VkResult VulkanContext::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(Instance, pCreateInfo, pAllocator, &_debugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanContext::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(Instance, _debugMessenger, pAllocator);
		}
	}


	//SURFACE

	void VulkanContext::CreateSurface(GLFWwindow* window)
	{
		if (glfwCreateWindowSurface(Instance, window, nullptr, &Surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	ViewProjectionUBO* VulkanContext::GetViewProjectionUBO()
	{
		return (ViewProjectionUBO*)ViewProjectionUniformBuffer->UniformBuffersMapped[CurrentFrame];
	}

	InstanceDataUBO* VulkanContext::GetInstanceDataUBO()
	{
		return (InstanceDataUBO*)InstanceDataUniformBuffer->UniformBuffersMapped[CurrentFrame];
	}

	//RENDER PASS


	//void VulkanContext::CreateRenderPass2(VulkanPhysicalDevice* physicalDevice,
	//	VulkanDevice* device,
	//	VkFormat& swapChainImageFormat)
	//{
	//	VkAttachmentDescription attachment = {};
	//	attachment.format = swapChainImageFormat; //wd->SurfaceFormat.format;
	//	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	//	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//	VkAttachmentReference color_attachment = {};
	//	color_attachment.attachment = 0;
	//	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//	VkSubpassDescription subpass = {};
	//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//	subpass.colorAttachmentCount = 1;
	//	subpass.pColorAttachments = &color_attachment;

	//	VkSubpassDependency dependency = {};
	//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	//	dependency.dstSubpass = 0;
	//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	//	dependency.srcAccessMask = 0;
	//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//	VkRenderPassCreateInfo info = {};
	//	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//	info.attachmentCount = 1;
	//	info.pAttachments = &attachment;
	//	info.subpassCount = 1;
	//	info.pSubpasses = &subpass;
	//	info.dependencyCount = 1;
	//	info.pDependencies = &dependency;

	//	if (vkCreateRenderPass(device->Device, &info, nullptr, &RenderPass) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("failed to create render pass!");
	//	}


	//	/*VkAttachmentDescription colorAttachment{};
	//	colorAttachment.format = swapChainImageFormat;
	//	colorAttachment.samples = physicalDevice->MsaaSamples;
	//	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//	VkAttachmentDescription depthAttachment{};
	//	depthAttachment.format = FindDepthFormat(physicalDevice->Device);
	//	depthAttachment.samples = physicalDevice->MsaaSamples;
	//	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//	VkAttachmentDescription colorAttachmentResolve{};
	//	colorAttachmentResolve.format = swapChainImageFormat;
	//	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	//	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	//	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//	VkAttachmentReference colorAttachmentRef{};
	//	colorAttachmentRef.attachment = 0;
	//	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//	VkAttachmentReference depthAttachmentRef{};
	//	depthAttachmentRef.attachment = 1;
	//	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//	VkAttachmentReference colorAttachmentResolveRef{};
	//	colorAttachmentResolveRef.attachment = 2;
	//	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//	VkSubpassDescription subpass{};
	//	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	//	subpass.colorAttachmentCount = 1;
	//	subpass.pColorAttachments = &colorAttachmentRef;
	//	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	//	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	//	VkSubpassDependency dependency{};
	//	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	//	dependency.dstSubpass = 0;
	//	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//	dependency.srcAccessMask = 0;
	//	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	//	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	//	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	//	VkRenderPassCreateInfo renderPassInfo{};
	//	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	//	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	//	renderPassInfo.pAttachments = attachments.data();
	//	renderPassInfo.subpassCount = 1;
	//	renderPassInfo.pSubpasses = &subpass;
	//	renderPassInfo.dependencyCount = 1;
	//	renderPassInfo.pDependencies = &dependency;

	//	if (vkCreateRenderPass(device->Device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("failed to create render pass!");
	//	}*/
	//}

	//void VulkanContext::CreateImguiFrameBuffer()
	//{
	//	// Create The Image Views
	//	{
	//		VkImageViewCreateInfo info = {};
	//		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	//		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	//		info.format = wd->SurfaceFormat.format;
	//		info.components.r = VK_COMPONENT_SWIZZLE_R;
	//		info.components.g = VK_COMPONENT_SWIZZLE_G;
	//		info.components.b = VK_COMPONENT_SWIZZLE_B;
	//		info.components.a = VK_COMPONENT_SWIZZLE_A;
	//		VkImageSubresourceRange image_range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	//		info.subresourceRange = image_range;
	//		for (uint32_t i = 0; i < wd->ImageCount; i++)
	//		{
	//			ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
	//			info.image = fd->Backbuffer;

	//			if (vkCreateImageView(Device->Device, &info, nullptr, &fd->BackbufferView) != VK_SUCCESS)
	//			{
	//				throw std::runtime_error("failed to CreateImguiFrameBuffer!");
	//			}
	//				

	//		}
	//	}

	//	// Create Framebuffer
	//	{
	//		VkImageView attachment[1]; //original
	//		//VkImageView attachment[3];
	//		VkFramebufferCreateInfo info = {};
	//		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//		info.renderPass = RenderPassImgui; //wd->RenderPass;
	//		info.attachmentCount = 1; //original
	//		//info.attachmentCount = 3;
	//		info.pAttachments = attachment;
	//		info.width = wd->Width;
	//		info.height = wd->Height;
	//		info.layers = 1;
	//		for (uint32_t i = 0; i < wd->ImageCount; i++)
	//		{
	//			ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
	//			attachment[0] = fd->BackbufferView;
	//			/*err = vkCreateFramebuffer(device, &info, allocator, &fd->Framebuffer);
	//			check_vk_result(err);*/

	//			if (vkCreateFramebuffer(Device->Device, &info, nullptr, &fd->Framebuffer) != VK_SUCCESS)
	//			{
	//				throw std::runtime_error("failed to CreateImguiFrameBuffer!");
	//			}
	//		}
	//	}
	//}

	void VulkanContext::CreateRenderPass(VulkanPhysicalDevice* physicalDevice,
		VulkanDevice* device,
		VkFormat& swapChainImageFormat)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = physicalDevice->MsaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindDepthFormat(physicalDevice->Device);
		depthAttachment.samples = physicalDevice->MsaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device->Device, &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}


	void VulkanContext::CreateImguiRenderPass(VulkanPhysicalDevice* physicalDevice,
		VulkanDevice* device,
		VkFormat& swapChainImageFormat)
	{
		VkAttachmentDescription attachment = {};
		attachment.format = VK_FORMAT_B8G8R8A8_SRGB; // wd->SurfaceFormat.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //wd->ClearEnable ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;

		/*err = vkCreateRenderPass(device, &info, allocator, &wd->RenderPass);
		check_vk_result(err);*/

		if (vkCreateRenderPass(device->Device, &info, nullptr, &RenderPassImgui) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		}

		


		//// Render Pass
		//VkAttachmentDescription colorAttachmentDesc = {};
		//// Color attachment
		//colorAttachmentDesc.format = swapChainImageFormat;
		//colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		//colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		//colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		//colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//VkAttachmentReference colorReference = {};
		//colorReference.attachment = 0;
		//colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//VkAttachmentReference depthReference = {};
		//depthReference.attachment = 1;
		//depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//VkSubpassDescription subpassDescription = {};
		//subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		//subpassDescription.colorAttachmentCount = 1;
		//subpassDescription.pColorAttachments = &colorReference;
		//subpassDescription.inputAttachmentCount = 0;
		//subpassDescription.pInputAttachments = nullptr;
		//subpassDescription.preserveAttachmentCount = 0;
		//subpassDescription.pPreserveAttachments = nullptr;
		//subpassDescription.pResolveAttachments = nullptr;

		//VkSubpassDependency dependency = {};
		//dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		//dependency.dstSubpass = 0;
		//dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		//dependency.srcAccessMask = 0;
		//dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		//VkRenderPassCreateInfo renderPassInfo = {};
		//renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		//renderPassInfo.attachmentCount = 1;
		//renderPassInfo.pAttachments = &colorAttachmentDesc;
		//renderPassInfo.subpassCount = 1;
		//renderPassInfo.pSubpasses = &subpassDescription;
		//renderPassInfo.dependencyCount = 1;
		//renderPassInfo.pDependencies = &dependency;

		//if (vkCreateRenderPass(device->Device, &renderPassInfo, nullptr, &RenderPassImgui) != VK_SUCCESS)
		//{
		//	throw std::runtime_error("failed to create render pass!");
		//}
	}

	VkFormat VulkanContext::FindDepthFormat(VkPhysicalDevice physicalDevice)
	{
		return FindSupportedFormat(physicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat VulkanContext::FindSupportedFormat(VkPhysicalDevice physicalDevice,
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	void VulkanContext::CreateDescriptorSetLayout(/*const std::vector<VulkanUniformBuffer*>& uniformBuffers*/)
	{
		size_t uniformBuffersCount = UniformBuffers.size();

		std::vector< VkDescriptorSetLayoutBinding> bindings;
		for (const VulkanUniformBuffer* ub : UniformBuffers)
		{
			bindings.push_back(ub->DescriptorSetLayout);
		}

		//hardcoded UBO, not great
		/*VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;*/

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = uniformBuffersCount; //1; //next available binding after uniform buffers
		samplerLayoutBinding.descriptorCount = _maxTextures; //1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings.push_back(samplerLayoutBinding);

		//old bindings using array
		//std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(Device->Device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	VkShaderModule VulkanContext::CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(Device->Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void VulkanContext::CreateCommandPool()
	{
		VulkanQueueFamilyIndices queueFamilyIndices = PhysicalDevice->QueueIndices;

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		if (vkCreateCommandPool(Device->Device, &poolInfo, nullptr, &CommandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics command pool!");
		}
	}

	void VulkanContext::CreateCommandBuffers()
	{
		CommandBuffers.resize(MaxFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)CommandBuffers.size();

		if (vkAllocateCommandBuffers(Device->Device, &allocInfo, CommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void VulkanContext::CreateCommandBuffer(VkCommandBuffer buffer)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(Device->Device, &allocInfo, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	VkCommandBuffer VulkanContext::CreateSecondaryCommandBuffer(const char* debugName)
	{
		VkCommandBuffer cmdBuffer;

		VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.commandPool = CommandPool; //Commandpool; //GetOrCreateThreadLocalCommandPool()->GetGraphicsCommandPool();
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmdBufAllocateInfo.commandBufferCount = 1;		

		VK_CHECK_RESULT(vkAllocateCommandBuffers(Device->Device, &cmdBufAllocateInfo, &cmdBuffer));
		VKUtils::SetDebugUtilsObjectName(Device->Device, VK_OBJECT_TYPE_COMMAND_BUFFER, debugName, cmdBuffer);
		return cmdBuffer;
	}
}
