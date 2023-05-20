#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	enum class EImageFormat
	{
		None			= VK_FORMAT_UNDEFINED,
		/* Color formats */
		R32F			= VK_FORMAT_R32_SFLOAT,
		R32I			= VK_FORMAT_R32_UINT,
		R16G16F			= VK_FORMAT_UNDEFINED,
		R32G32F			= VK_FORMAT_UNDEFINED,
		R8G8B8			= VK_FORMAT_R8G8B8_UNORM,
		R8G8B8A8		= VK_FORMAT_R8G8B8A8_UNORM,
		R16G16B16A16F	= VK_FORMAT_R16G16B16A16_SFLOAT,
		R32G32B32A32F	= VK_FORMAT_R32G32B32A32_SFLOAT,
		SRGB			= VK_FORMAT_UNDEFINED,
		/* Depth formats */
		Depth32			= VK_FORMAT_D32_SFLOAT,
		Depth24Stencil8 = VK_FORMAT_D24_UNORM_S8_UINT,
		Depth32Stencil8 = VK_FORMAT_D32_SFLOAT_S8_UINT,
		/* Defaults */
		Depth			= Depth24Stencil8,
	};

	uint32_t ChannelCountFromFormat(const EImageFormat format);
	bool FormatHasAlphaChannel(const VkFormat format);

	void InsertImageMemoryBarrier(
		const VkCommandBuffer commandBuffer,
		const VkImage image,
		const VkAccessFlags srcAccessMask,
		const VkAccessFlags dstAccessMask,
		const VkImageLayout oldImageLayout,
		const VkImageLayout newImageLayout,
		const VkPipelineStageFlags srcStageMask,
		const VkPipelineStageFlags dstStageMask,
		const VkImageSubresourceRange subresourceRange);
}