#pragma once
#include "VulkanQueueFamilyIndices.h"
#include <vector>
#include <set>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include "VulkanPhysicalDevice.h"

namespace Kanto
{
	class VulkanDevice
	{
	public:
		VkDevice Device;
		VkQueue GraphicsQueue;
		VkQueue PresentQueue;
		VulkanDevice(bool enableValidationLayers,
			const std::vector<const char*> validationLayers,
			VulkanPhysicalDevice& physicalDevice);
		~VulkanDevice();
	};


}
