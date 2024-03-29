#pragma once

#include "VulkanUtils.h"

namespace Kanto::Utils 
{

	struct VulkanCheckpointData
	{
		char Data[64 + 1]{};
	};

	void SetVulkanCheckpoint(VkCommandBuffer commandBuffer, const std::string& data);

}

