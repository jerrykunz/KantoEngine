#include "knpch.h"
#include "VulkanAPI.h"

#include "VulkanContext.h"

#include "Kanto/Renderer/RendererStats.h"
#include "VulkanUtils.h"

namespace Kanto
{

	VkDescriptorSetAllocateInfo DescriptorSetAllocInfo(const VkDescriptorSetLayout* layouts, uint32_t count, VkDescriptorPool pool)
	{
		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.pSetLayouts = layouts;
		info.descriptorSetCount = count;
		info.descriptorPool = pool;
		return info;
	}

	VkSampler CreateSampler(VkSamplerCreateInfo samplerCreateInfo)
	{
		auto device = VulkanContext::Get()->Device->Device; //VulkanContext::GetCurrentDevice();
		VkDevice vulkanDevice = device; //device->GetVulkanDevice();

		VkSampler sampler;
		VK_CHECK_RESULT(vkCreateSampler(vulkanDevice, &samplerCreateInfo, nullptr, &sampler));

		RendererUtils::GetResourceAllocationCounts().Samplers++;

		return sampler;
	}

	void DestroySampler(VkSampler sampler)
	{
		auto device = VulkanContext::Get()->Device->Device; //VulkanContext::GetCurrentDevice();
		VkDevice vulkanDevice = device; //device->GetVulkanDevice();
		vkDestroySampler(vulkanDevice, sampler, nullptr);

		RendererUtils::GetResourceAllocationCounts().Samplers--;
	}

}
