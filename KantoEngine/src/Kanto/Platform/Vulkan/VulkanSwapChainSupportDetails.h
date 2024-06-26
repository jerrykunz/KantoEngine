#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>


namespace Kanto
{
	struct VulkanSwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
}
