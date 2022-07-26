#pragma once
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Renderer/VulkanTypes.h"

namespace Cinnamon {
	class GraphicsContext
	{
	public:
		enum class EQueueFamily
		{
			Graphics,
			Compute,
			Transfer,
			Present,
		};
	private:
		GraphicsContext() noexcept = delete;
		~GraphicsContext() noexcept = delete;
	public:
		static bool [[nodiscard]] Initialize();
		static bool [[nodiscard]] Shutdown();
		static bool [[nodiscard]] CreateSurface(Window* windowContext);

		static void AcquireNextImage(Window* windowContext);
		static void PresentImage(Window* windowContext);

		static CIN_FORCE_INLINE constexpr VkInstance GetInstance()
		{
			return s_Instance;
		}

		static CIN_FORCE_INLINE constexpr VkSurfaceKHR GetSurface()
		{
			return s_Surface;
		}

		static CIN_FORCE_INLINE constexpr VkPhysicalDevice GetPhysicalDevice()
		{
			return s_PhysicalDevice;
		}

		static CIN_FORCE_INLINE constexpr VkDevice GetDevice()
		{
			return s_LogicalDevice;
		}

		//static CIN_FORCE_INLINE constexpr Swapchain* GetSwapchain()
		//{
		//	return s_Swapchain;
		//}

		static CIN_FORCE_INLINE constexpr VkDebugReportCallbackEXT GetDebugReportCallback()
		{
#ifdef CIN_DEBUG
			return s_DebugObject;
#else
			return nullptr;
#endif
		}

		static CIN_FORCE_INLINE consteval VkAllocationCallbacks* GetAllocator()
		{
			return s_Allocator;
		}

		static CIN_FORCE_INLINE int32_t GetQueueFamily(const EQueueFamily queueFamily)
		{
			switch (queueFamily)
			{
				case EQueueFamily::Graphics:	return s_QueueFamilies.Graphics;
				case EQueueFamily::Compute:		return s_QueueFamilies.Compute;
				case EQueueFamily::Transfer:	return s_QueueFamilies.Transfer;
				case EQueueFamily::Present:		return s_QueueFamilies.Present;
			}

			CIN_ASSERT(false, "Unknown family");
			return -1;
		}

		static CIN_FORCE_INLINE constexpr VkQueue GetGraphicsQueue()
		{
			return s_Queues.Graphics;
		}

		static CIN_FORCE_INLINE constexpr VkQueue GetPresentQueue()
		{
			return s_Queues.Present;
		}

		static CIN_FORCE_INLINE constexpr VkCommandPool GetGraphicsCommandPool()
		{
			return s_CommandPools.Graphics;
		}
	private:
		static inline VkInstance s_Instance{ VK_NULL_HANDLE };
		static inline VkSurfaceKHR s_Surface{ VK_NULL_HANDLE };
		static inline VkPhysicalDevice s_PhysicalDevice{ VK_NULL_HANDLE };
		static inline VkDevice s_LogicalDevice{ VK_NULL_HANDLE };
		//static inline Swapchain* s_Swapchain{ VK_NULL_HANDLE };
#ifdef CIN_DEBUG
		static inline VkDebugReportCallbackEXT s_DebugObject{ VK_NULL_HANDLE };
#endif
		static inline constexpr VkAllocationCallbacks* s_Allocator{ VK_NULL_HANDLE };

		struct QueueFamilies
		{
			int32_t Graphics{ -1 };
			int32_t Compute{ -1 };
			int32_t Transfer{ -1 };
			int32_t Present{ -1 };
		} static inline s_QueueFamilies;

		struct Queues {
			VkQueue Graphics{ VK_NULL_HANDLE };
			VkQueue Compute{ VK_NULL_HANDLE };
			VkQueue Transfer{ VK_NULL_HANDLE };
			VkQueue Present{ VK_NULL_HANDLE };
		} static inline s_Queues;

		struct CommandPools {
			VkCommandPool Graphics{ VK_NULL_HANDLE };
			VkCommandPool Compute{ VK_NULL_HANDLE };
			VkCommandPool Transfer{ VK_NULL_HANDLE };
			VkCommandPool Present{ VK_NULL_HANDLE };
		} static inline s_CommandPools;
	};
}