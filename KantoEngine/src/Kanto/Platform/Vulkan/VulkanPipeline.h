#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>

namespace Kanto
{
	class VulkanPipeline
	{
	private:
		std::vector<char> ReadFile(const std::string& filename);
		VkShaderModule CreateShaderModule(VkDevice& device, const std::vector<char>& code);

	public:
		VkPipeline Pipeline;
		VkPipelineLayout PipelineLayout;

		VulkanPipeline(VkDevice& device,
			VkRenderPass& renderPass,
			VkSampleCountFlagBits& msaaSamples,
			VkDescriptorSetLayout& descriptorSetLayout,
			const std::string& vertShader,
			const std::string& fragShader,
			const VkVertexInputBindingDescription& bindingDescription,
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions,
			VkPrimitiveTopology topology,
			VkPolygonMode polygonMode);
		void Dispose(VkDevice& device);
	};


}
