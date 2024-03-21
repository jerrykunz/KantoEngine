#include "knpch.h"
#include "VulkanVertexBuffer.h"

namespace Kanto
{
	VulkanVertexBuffer::VulkanVertexBuffer()
	{
		VertexBuffer = VK_NULL_HANDLE;
	}

	void VulkanVertexBuffer::Dispose(VkDevice& device)
	{
		vkDestroyBuffer(device, VertexBuffer, nullptr);
		vkFreeMemory(device, VertexBufferMemory, nullptr);
	}
}
