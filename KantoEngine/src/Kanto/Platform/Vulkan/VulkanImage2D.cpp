#include "knpch.h"
//#include "VulkanImage2D.h"
//
//#include "VulkanContext.h"
//#include "VulkanAPI.h"
//#include "VulkanUtils.h"
////#include "VulkanRenderer.h"
////#include "VulkanAPI.h"
//
//namespace Kanto 
//{
//
//	static std::map<VkImage, WeakRef<VulkanImage2D>> s_ImageReferences;
//
//	VulkanImage2D::VulkanImage2D(const ImageSpecification& specification)
//		: m_Specification(specification)
//	{
//		KN_CORE_VERIFY(m_Specification.Width > 0 && m_Specification.Height > 0);
//	}
//
//	VulkanImage2D::~VulkanImage2D()
//	{
//		Release();
//	}
//
//	void VulkanImage2D::Invalidate()
//	{
//		/*Ref<VulkanImage2D> instance = this;
//		Renderer::Submit([instance]() mutable
//			{
//				instance->RT_Invalidate();
//			});*/
//
//		RT_Invalidate();
//	}
//
//	void VulkanImage2D::Release()
//	{
//		if (m_Info.Image == nullptr)
//			return;
//
//		const VulkanImageInfo& info = m_Info;
//		/*Renderer::SubmitResourceFree([info, mipViews = m_PerMipImageViews, layerViews = m_PerLayerImageViews]() mutable
//			{
//				const auto vulkanDevice = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
//				vkDestroyImageView(vulkanDevice, info.ImageView, nullptr);
//				Vulkan::DestroySampler(info.Sampler);
//
//				for (auto& view : mipViews)
//				{
//					if (view.second)
//						vkDestroyImageView(vulkanDevice, view.second, nullptr);
//				}
//				for (auto& view : layerViews)
//				{
//					if (view)
//						vkDestroyImageView(vulkanDevice, view, nullptr);
//				}
//				VulkanAllocator allocator("VulkanImage2D");
//				allocator.DestroyImage(info.Image, info.MemoryAlloc);
//				s_ImageReferences.erase(info.Image);
//			});*/
//
//		const auto vulkanDevice = VulkanContext::Get()->Device->Device;
//		vkDestroyImageView(vulkanDevice, info.ImageView, nullptr);
//		DestroySampler(info.Sampler);
//
//		for (auto& view : m_PerMipImageViews)
//		{
//			if (view.second)
//				vkDestroyImageView(vulkanDevice, view.second, nullptr);
//		}
//		for (auto& view : m_PerLayerImageViews)
//		{
//			if (view)
//				vkDestroyImageView(vulkanDevice, view, nullptr);
//		}
//		/*VulkanAllocator allocator("VulkanImage2D");
//		allocator.DestroyImage(info.Image, info.MemoryAlloc);*/
//		s_ImageReferences.erase(info.Image);
//
//
//
//		m_Info.Image = nullptr;
//		m_Info.ImageView = nullptr;
//		if (m_Specification.CreateSampler)
//			m_Info.Sampler = nullptr;
//		m_PerLayerImageViews.clear();
//		m_PerMipImageViews.clear();
//
//	}
//
//	void VulkanImage2D::RT_Invalidate()
//	{
//		KN_CORE_VERIFY(m_Specification.Width > 0 && m_Specification.Height > 0);
//
//		// Try release first if necessary
//		Release();
//
//		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
//
//		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: this (probably) shouldn't be implied
//		if (m_Specification.Usage == ImageUsage::Attachment)
//		{
//			if (Utils::IsDepthFormat(m_Specification.Format))
//				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
//			else
//				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//		}
//		if (m_Specification.Transfer || m_Specification.Usage == ImageUsage::Texture)
//		{
//			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//		}
//		if (m_Specification.Usage == ImageUsage::Storage)
//		{
//			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//		}
//
//		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
//		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
//			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//
//		VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);
//
//		VkImageCreateInfo imageCreateInfo = {};
//		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
//		imageCreateInfo.format = vulkanFormat;
//		imageCreateInfo.extent.width = m_Specification.Width;
//		imageCreateInfo.extent.height = m_Specification.Height;
//		imageCreateInfo.extent.depth = 1;
//		imageCreateInfo.mipLevels = m_Specification.Mips;
//		imageCreateInfo.arrayLayers = m_Specification.Layers;
//		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//		imageCreateInfo.tiling = m_Specification.Usage == ImageUsage::HostRead ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
//		imageCreateInfo.usage = usage;
//
//		VkResult result = vkCreateImage(device, &imageCreateInfo, nullptr, &m_Info.Image);
//		KN_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan image!");
//
//		VkMemoryRequirements memRequirements;
//		vkGetImageMemoryRequirements(device, m_Info.Image, &memRequirements);
//
//		/*VkMemoryAllocateInfo allocInfo = {};
//		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		allocInfo.allocationSize = memRequirements.size;
//		allocInfo.memoryTypeIndex = VulkanContext::GetCurrentDevice()->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//
//		result = vkAllocateMemory(device, &allocInfo, nullptr, &m_Info.ImageMemory);
//		HZ_CORE_ASSERT(result == VK_SUCCESS, "Failed to allocate Vulkan memory!");
//
//		vkBindImageMemory(device, m_Info.Image, m_Info.ImageMemory, 0);*/
//
//		VkMemoryRequirements memRequirements;
//		vkGetImageMemoryRequirements(device, m_Info.Image, &memRequirements);
//
//		VkMemoryAllocateInfo allocInfo{};
//		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//		allocInfo.allocationSize = memRequirements.size;
//
//		//func
//		auto properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
//		auto devicee = VulkanContext::Get()->PhysicalDevice->Device;
//		VkPhysicalDeviceMemoryProperties memProperties;
//		vkGetPhysicalDeviceMemoryProperties(devicee, &memProperties);
//
//		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
//		{
//			if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
//			{
//				allocInfo.memoryTypeIndex = i;
//				break;
//			}
//		}
//
//		throw std::runtime_error("failed to find suitable memory type!");
//		//end
//
//
//		//allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
//
//		if (vkAllocateMemory(device, &allocInfo, nullptr,&m_Info.ImageMemory ) != VK_SUCCESS) {
//			throw std::runtime_error("failed to allocate image memory!");
//		}
//
//		vkBindImageMemory(device, m_Info.Image, imageMemory, 0);
//
//
//		m_GPUAllocationSize = allocInfo.allocationSize;
//
//		s_ImageReferences[m_Info.Image] = this;
//		VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE, m_Specification.DebugName, m_Info.Image);
//
//		// Create a default image view
//		VkImageViewCreateInfo imageViewCreateInfo = {};
//		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		imageViewCreateInfo.viewType = m_Specification.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
//		imageViewCreateInfo.format = vulkanFormat;
//		imageViewCreateInfo.flags = 0;
//		imageViewCreateInfo.subresourceRange = {};
//		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
//		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//		imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
//		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
//		imageViewCreateInfo.subresourceRange.layerCount = m_Specification.Layers;
//		imageViewCreateInfo.image = m_Info.Image;
//
//		result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_Info.ImageView);
//		HZ_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan image view!");
//		VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW, fmt::format("{} default image view", m_Specification.DebugName), m_Info.ImageView);
//
//		// TODO: Renderer should contain some kind of sampler cache
//		if (m_Specification.CreateSampler)
//		{
//			VkSamplerCreateInfo samplerCreateInfo = {};
//			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//			samplerCreateInfo.maxAnisotropy = 1.0f;
//			if (Utils::IsIntegerBased(m_Specification.Format))
//			{
//				samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
//				samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
//				samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
//			}
//			else
//			{
//				samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
//				samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
//				samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//			}
//
//			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
//			samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
//			samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
//			samplerCreateInfo.mipLodBias = 0.0f;
//			samplerCreateInfo.minLod = 0.0f;
//			samplerCreateInfo.maxLod = 100.0f;
//			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
//
//			result = vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_Info.Sampler);
//			HZ_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan sampler!");
//			VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_SAMPLER, fmt::format("{} default sampler", m_Specification.DebugName), m_Info.Sampler);
//		}
//
//		if (m_Specification.Usage == ImageUsage::Storage)
//		{
//			// Transition image to GENERAL layout
//			VkCommandBuffer commandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(false);
//
//			VkImageSubresourceRange subresourceRange = {};
//			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			subresourceRange.baseMipLevel = 0;
//			subresourceRange.levelCount = m_Specification.Mips;
//			subresourceRange.layerCount = m_Specification.Layers;
//
//			Utils::InsertImageMemoryBarrier(commandBuffer, m_Info.Image,
//				0, 0,
//				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
//				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//				subresourceRange);
//
//			VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
//		}
//		else if (m_Specification.Usage == ImageUsage::HostRead)
//		{
//			// Transition image to TRANSFER_DST layout
//			VkCommandBuffer commandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(false);
//
//			VkImageSubresourceRange subresourceRange = {};
//			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			subresourceRange.baseMipLevel = 0;
//			subresourceRange.levelCount = m_Specification.Mips;
//			subresourceRange.layerCount = m_Specification.Layers;
//
//			Utils::InsertImageMemoryBarrier(commandBuffer, m_Info.Image,
//				0, 0,
//				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
//				subresourceRange);
//
//			VulkanContext::GetCurrentDevice()->FlushCommandBuffer(commandBuffer);
//		}
//
//		UpdateDescriptor();
//	}
//
//	void VulkanImage2D::CreatePerLayerImageViews()
//	{
//		Ref<VulkanImage2D> instance = this;
//		Renderer::Submit([instance]() mutable
//			{
//				instance->RT_CreatePerLayerImageViews();
//			});
//	}
//
//	void VulkanImage2D::RT_CreatePerLayerImageViews()
//	{
//		KN_CORE_ASSERT(m_Specification.Layers > 1);
//
//		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
//
//		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
//		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
//			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//
//		const VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);
//
//		m_PerLayerImageViews.resize(m_Specification.Layers);
//		for (uint32_t layer = 0; layer < m_Specification.Layers; layer++)
//		{
//			VkImageViewCreateInfo imageViewCreateInfo = {};
//			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//			imageViewCreateInfo.format = vulkanFormat;
//			imageViewCreateInfo.flags = 0;
//			imageViewCreateInfo.subresourceRange = {};
//			imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
//			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//			imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
//			imageViewCreateInfo.subresourceRange.baseArrayLayer = layer;
//			imageViewCreateInfo.subresourceRange.layerCount = 1;
//			imageViewCreateInfo.image = m_Info.Image;
//			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerLayerImageViews[layer]));
//			VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW, fmt::format("{} image view layer: {}", m_Specification.DebugName, layer), m_PerLayerImageViews[layer]);
//		}
//	}
//
//	VkImageView VulkanImage2D::GetMipImageView(uint32_t mip)
//	{
//		if (m_PerMipImageViews.find(mip) == m_PerMipImageViews.end())
//		{
//			Ref<VulkanImage2D> instance = this;
//			Renderer::Submit([instance, mip]() mutable
//				{
//					instance->RT_GetMipImageView(mip);
//				});
//			return nullptr;
//		}
//
//		return m_PerMipImageViews.at(mip);
//	}
//
//	VkImageView VulkanImage2D::RT_GetMipImageView(const uint32_t mip)
//	{
//		auto it = m_PerMipImageViews.find(mip);
//		if (it != m_PerMipImageViews.end())
//			return it->second;
//
//		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
//
//		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
//		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
//			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//
//		VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);
//
//		VkImageViewCreateInfo imageViewCreateInfo = {};
//		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//		imageViewCreateInfo.format = vulkanFormat;
//		imageViewCreateInfo.flags = 0;
//		imageViewCreateInfo.subresourceRange = {};
//		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
//		imageViewCreateInfo.subresourceRange.baseMipLevel = mip;
//		imageViewCreateInfo.subresourceRange.levelCount = 1;
//		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
//		imageViewCreateInfo.subresourceRange.layerCount = 1;
//		imageViewCreateInfo.image = m_Info.Image;
//
//		VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerMipImageViews[mip]));
//		VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW, fmt::format("{} image view mip: {}", m_Specification.DebugName, mip), m_PerMipImageViews[mip]);
//		return m_PerMipImageViews.at(mip);
//	}
//
//	void VulkanImage2D::RT_CreatePerSpecificLayerImageViews(const std::vector<uint32_t>& layerIndices)
//	{
//		KN_CORE_ASSERT(m_Specification.Layers > 1);
//
//		VkDevice device = VulkanContext::GetCurrentDevice()->GetVulkanDevice();
//
//		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
//		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
//			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//
//		const VkFormat vulkanFormat = Utils::VulkanImageFormat(m_Specification.Format);
//
//		//KN_CORE_ASSERT(m_PerLayerImageViews.size() == m_Specification.Layers);
//		if (m_PerLayerImageViews.empty())
//			m_PerLayerImageViews.resize(m_Specification.Layers);
//
//		for (uint32_t layer : layerIndices)
//		{
//			VkImageViewCreateInfo imageViewCreateInfo = {};
//			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//			imageViewCreateInfo.format = vulkanFormat;
//			imageViewCreateInfo.flags = 0;
//			imageViewCreateInfo.subresourceRange = {};
//			imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
//			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
//			imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
//			imageViewCreateInfo.subresourceRange.baseArrayLayer = layer;
//			imageViewCreateInfo.subresourceRange.layerCount = 1;
//			imageViewCreateInfo.image = m_Info.Image;
//			VK_CHECK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_PerLayerImageViews[layer]));
//			VKUtils::SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_IMAGE_VIEW, fmt::format("{} image view layer: {}", m_Specification.DebugName, layer), m_PerLayerImageViews[layer]);
//		}
//
//	}
//
//	void VulkanImage2D::UpdateDescriptor()
//	{
//		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8 || m_Specification.Format == ImageFormat::DEPTH32F || m_Specification.Format == ImageFormat::DEPTH32FSTENCIL8UINT)
//			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
//		else if (m_Specification.Usage == ImageUsage::Storage)
//			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//		else
//			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//
//		if (m_Specification.Usage == ImageUsage::Storage)
//			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
//		else if (m_Specification.Usage == ImageUsage::HostRead)
//			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//
//		m_DescriptorImageInfo.imageView = m_Info.ImageView;
//		m_DescriptorImageInfo.sampler = m_Info.Sampler;
//
//		//KN_CORE_WARN_TAG("Renderer", "VulkanImage2D::UpdateDescriptor to ImageView = {0}", (const void*)m_Info.ImageView);
//	}
//
//	const std::map<VkImage, WeakRef<VulkanImage2D>>& VulkanImage2D::GetImageRefs()
//	{
//		return s_ImageReferences;
//	}
//
//	void VulkanImage2D::CopyToHostBuffer(Buffer& buffer)
//	{
//		auto device = VulkanContext::GetCurrentDevice();
//		auto vulkanDevice = device->GetVulkanDevice();
//		VulkanAllocator allocator("Image2D");
//
//		uint64_t bufferSize = m_Specification.Width * m_Specification.Height * Utils::GetImageFormatBPP(m_Specification.Format);
//
//		// Create staging buffer
//		VkBufferCreateInfo bufferCreateInfo{};
//		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//		bufferCreateInfo.size = bufferSize;
//		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//		VkBuffer stagingBuffer;
//		VmaAllocation stagingBufferAllocation = allocator.AllocateBuffer(bufferCreateInfo, VMA_MEMORY_USAGE_GPU_TO_CPU, stagingBuffer);
//
//		uint32_t mipCount = 1;
//		uint32_t mipWidth = m_Specification.Width, mipHeight = m_Specification.Height;
//
//		VkCommandBuffer copyCmd = device->GetCommandBuffer(true);
//
//		VkImageSubresourceRange subresourceRange = {};
//		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		subresourceRange.baseMipLevel = 0;
//		subresourceRange.levelCount = mipCount;
//		subresourceRange.layerCount = 1;
//
//		Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
//			VK_ACCESS_TRANSFER_READ_BIT, 0,
//			m_DescriptorImageInfo.imageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
//			subresourceRange);
//
//		uint64_t mipDataOffset = 0;
//		for (uint32_t mip = 0; mip < mipCount; mip++)
//		{
//			VkBufferImageCopy bufferCopyRegion = {};
//			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			bufferCopyRegion.imageSubresource.mipLevel = mip;
//			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
//			bufferCopyRegion.imageSubresource.layerCount = 1;
//			bufferCopyRegion.imageExtent.width = mipWidth;
//			bufferCopyRegion.imageExtent.height = mipHeight;
//			bufferCopyRegion.imageExtent.depth = 1;
//			bufferCopyRegion.bufferOffset = mipDataOffset;
//
//			vkCmdCopyImageToBuffer(
//				copyCmd,
//				m_Info.Image,
//				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//				stagingBuffer,
//				1,
//				&bufferCopyRegion);
//
//			uint64_t mipDataSize = mipWidth * mipHeight * sizeof(float) * 4 * 6;
//			mipDataOffset += mipDataSize;
//			mipWidth /= 2;
//			mipHeight /= 2;
//		}
//
//		Utils::InsertImageMemoryBarrier(copyCmd, m_Info.Image,
//			VK_ACCESS_TRANSFER_READ_BIT, 0,
//			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_DescriptorImageInfo.imageLayout,
//			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//			subresourceRange);
//
//		device->FlushCommandBuffer(copyCmd);
//
//		// Copy data from staging buffer
//		uint8_t* srcData = allocator.MapMemory<uint8_t>(stagingBufferAllocation);
//		buffer.Allocate(bufferSize);
//		memcpy(buffer.Data, srcData, bufferSize);
//		allocator.UnmapMemory(stagingBufferAllocation);
//
//		allocator.DestroyBuffer(stagingBuffer, stagingBufferAllocation);
//	}
//
//
//	VulkanImageView::VulkanImageView(const ImageViewSpecification& specification)
//		: m_Specification(specification)
//	{
//		Invalidate();
//	}
//
//	VulkanImageView::~VulkanImageView()
//	{
//		Renderer::SubmitResourceFree([imageView = m_ImageView]() mutable
//			{
//				auto device = VulkanContext::GetCurrentDevice();
//				VkDevice vulkanDevice = device->GetVulkanDevice();
//
//				vkDestroyImageView(vulkanDevice, imageView, nullptr);
//			});
//
//		m_ImageView = nullptr;
//	}
//
//	void VulkanImageView::Invalidate()
//	{
//		Ref<VulkanImageView> instance = this;
//		Renderer::Submit([instance]() mutable
//			{
//				instance->RT_Invalidate();
//			});
//	}
//
//
//	void VulkanImageView::RT_Invalidate()
//	{
//		auto device = VulkanContext::GetCurrentDevice();
//		VkDevice vulkanDevice = device->GetVulkanDevice();
//
//		Ref<VulkanImage2D> vulkanImage = m_Specification.Image.As<VulkanImage2D>();
//		const auto& imageSpec = vulkanImage->GetSpecification();
//
//		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(imageSpec.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
//		if (imageSpec.Format == ImageFormat::DEPTH24STENCIL8)
//			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//
//		VkFormat vulkanFormat = Utils::VulkanImageFormat(imageSpec.Format);
//
//		VkImageViewCreateInfo imageViewCreateInfo = {};
//		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//		imageViewCreateInfo.viewType = imageSpec.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
//		imageViewCreateInfo.format = vulkanFormat;
//		imageViewCreateInfo.flags = 0;
//		imageViewCreateInfo.subresourceRange = {};
//		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
//		imageViewCreateInfo.subresourceRange.baseMipLevel = m_Specification.Mip;
//		imageViewCreateInfo.subresourceRange.levelCount = 1;
//		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
//		imageViewCreateInfo.subresourceRange.layerCount = imageSpec.Layers;
//		imageViewCreateInfo.image = vulkanImage->GetImageInfo().Image;
//		VK_CHECK_RESULT(vkCreateImageView(vulkanDevice, &imageViewCreateInfo, nullptr, &m_ImageView));
//		VKUtils::SetDebugUtilsObjectName(vulkanDevice, VK_OBJECT_TYPE_IMAGE_VIEW, fmt::format("{} default image view", m_Specification.DebugName), m_ImageView);
//
//		m_DescriptorImageInfo = vulkanImage->GetDescriptorInfoVulkan();
//		m_DescriptorImageInfo.imageView = m_ImageView;
//	}
//
//}
