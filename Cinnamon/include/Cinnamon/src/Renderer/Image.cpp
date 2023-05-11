#include "Cinnamon/include/Renderer/Image.hpp"

namespace Cinnamon {
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