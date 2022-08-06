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
		[[nodiscard]] static bool Initialize();
		[[nodiscard]] static bool Shutdown();
		[[nodiscard]] static bool CreateSurface(const Window* const windowContext);
		static void ResizeSurface(const Window* const windowContext, const uint32_t width, const uint32_t height);

		static void AcquireNextImage(const Window* const windowContext);
		static void PresentImage(const Window* const windowContext);

		static CIN_FORCE_INLINE VkInstance GetInstance()
		{
			return s_Instance;
		}

		static CIN_FORCE_INLINE VkSurfaceKHR GetSurface()
		{
			return s_Surface;
		}

		static CIN_FORCE_INLINE VkPhysicalDevice GetPhysicalDevice()
		{
			return s_PhysicalDevice;
		}

		static CIN_FORCE_INLINE VkDevice GetDevice()
		{
			return s_LogicalDevice;
		}

		//static CIN_FORCE_INLINE constexpr Swapchain* GetSwapchain()
		//{
		//	return s_Swapchain;
		//}

		static CIN_FORCE_INLINE VkDebugReportCallbackEXT GetDebugReportCallback()
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

		static CIN_FORCE_INLINE VkQueue GetGraphicsQueue()
		{
			return s_Queues.Graphics;
		}

		static CIN_FORCE_INLINE VkQueue GetPresentQueue()
		{
			return s_Queues.Present;
		}

		static CIN_FORCE_INLINE VkCommandPool GetGraphicsCommandPool()
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
			int32_t Graphics;
			int32_t Compute;
			int32_t Transfer;
			int32_t Present;

            QueueFamilies()
            :
            Graphics(-1),
            Compute(-1),
            Transfer(-1),
            Present(-1)
            {}
		} static inline s_QueueFamilies;

		struct Queues {
			VkQueue Graphics;
			VkQueue Compute;
			VkQueue Transfer;
			VkQueue Present;

            Queues()
            :
            Graphics(VK_NULL_HANDLE),
            Compute(VK_NULL_HANDLE),
            Transfer(VK_NULL_HANDLE),
            Present(VK_NULL_HANDLE)
            {} 
		} static inline s_Queues;

		struct CommandPools {
			VkCommandPool Graphics;
			VkCommandPool Compute;
			VkCommandPool Transfer;
			VkCommandPool Present;

            CommandPools()
            :
            Graphics(VK_NULL_HANDLE),
            Compute(VK_NULL_HANDLE),
            Transfer(VK_NULL_HANDLE),
            Present(VK_NULL_HANDLE)
            {}
		} static inline s_CommandPools;
	};
}