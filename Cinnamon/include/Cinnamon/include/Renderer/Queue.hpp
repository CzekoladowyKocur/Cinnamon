#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	struct QueueFamilies final
	{
		enum : uint32_t
		{
			Absent = std::numeric_limits<uint32_t>::max(),
		};

		uint32_t Graphics{ Absent };
		uint32_t Compute{ Absent };
		uint32_t Transfer{ Absent };
		uint32_t Present{ Absent };

		uint32_t GraphicsQueueCount{ 0U };
		uint32_t ComputeQueueCount{ 0U };
		uint32_t TransferQueueCount{ 0U };
		uint32_t PresentQueueCount{ 0U };
	};

	struct Queues final
	{
		VkQueue Graphics{ VK_NULL_HANDLE };
		VkQueue Compute{ VK_NULL_HANDLE };
		VkQueue Transfer{ VK_NULL_HANDLE };
		VkQueue Present{ VK_NULL_HANDLE };
	};
}