#include "Cinnamon/include/Renderer/Image.hpp"

namespace Cinnamon {
	uint32_t ChannelCountFromFormat(const EImageFormat format)
	{
		switch (format)
		{
			case EImageFormat::R8G8B8:		return 3U;
			case EImageFormat::R8G8B8A8:	return 4U;

			[[unlikely]] default: CIN_ASSERT(false); return 4U;
		}
	}

	bool FormatHasAlphaChannel(const VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_R16G16B16A16_SFLOAT:
			case VK_FORMAT_R32G32B32A32_SFLOAT:
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_UNORM:
				return true;

			[[unlikely]] default: CIN_ASSERT(false); return false;
		}
	}

	void InsertImageMemoryBarrier(
		const VkCommandBuffer commandBuffer,
		const VkImage image,
		const VkAccessFlags srcAccessMask,
		const VkAccessFlags dstAccessMask,
		const VkImageLayout oldImageLayout,
		const VkImageLayout newImageLayout,
		const VkPipelineStageFlags srcStageMask,
		const VkPipelineStageFlags dstStageMask,
		const VkImageSubresourceRange subresourceRange)
	{
		CIN_ASSERT(commandBuffer, "Invalid command buffer");
		CIN_ASSERT(image, "Invalid image");

		const VkImageMemoryBarrier imageMemoryBarrier
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER },
			.pNext{ nullptr },
			.srcAccessMask{ srcAccessMask },
			.dstAccessMask{ dstAccessMask },
			.oldLayout{ oldImageLayout },
			.newLayout{ newImageLayout },
			.srcQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
			.dstQueueFamilyIndex{ VK_QUEUE_FAMILY_IGNORED },
			.image{ image },
			.subresourceRange{ subresourceRange },
		};

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStageMask,
			dstStageMask,
			0U,
			0U, nullptr, 
			0U, nullptr,
			1U, &imageMemoryBarrier);
	}
}