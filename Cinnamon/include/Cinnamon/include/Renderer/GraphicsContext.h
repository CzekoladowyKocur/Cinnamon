#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"

namespace Cinnamon {
	class Window;
	class Surface;
	class Swapchain;
}

namespace Cinnamon {
	class GraphicsContext
	{
	private:
		NON_CONSTRUCTIBLE(GraphicsContext)
		NON_COPYABLE(GraphicsContext)
	public:
		enum class EQueueFamily
		{
			Graphics = 0U,
			Compute = 1U,
			Transfer = 2U,
			Present = 3U,
		};
	public:
		[[nodiscard]] static bool Initialize();
		[[nodiscard]] static bool Shutdown();

		[[nodiscard]] static bool CreateSurface(const Window* const windowContext);
		static void RecreateSurface();
		static void ResizeSwapchain();

		static void AcquireNextImage();
		static void PresentImage();

		template<typename Function>
		static void PerformSingleSubmitMemoryTransferOperation(Function&& function)
		{
			CIN_ASSERT(s_CommandPools.Transfer, "Invalid transfer command pool");
			const VkCommandBufferAllocateInfo transferCommandBufferAllocateInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
				.pNext{ nullptr },
				.commandPool{ s_CommandPools.Transfer },
				.level{ VK_COMMAND_BUFFER_LEVEL_PRIMARY },
				.commandBufferCount{ 1U },
			};

			VkCommandBuffer transferCommandBuffer;
			VK_CHECK(vkAllocateCommandBuffers(
				s_LogicalDevice,
				&transferCommandBufferAllocateInfo,
				&transferCommandBuffer));

			const VkCommandBufferBeginInfo transferCommandBufferBeginInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
				.pNext{ nullptr },
				.flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT },
				.pInheritanceInfo{ nullptr },
			};

			VK_CHECK(vkBeginCommandBuffer(
				transferCommandBuffer,
				&transferCommandBufferBeginInfo));

			/* User code */
			function(transferCommandBuffer);

			VK_CHECK(vkEndCommandBuffer(
				transferCommandBuffer));

			const VkSubmitInfo submitInfo{
				.sType{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
				.pNext{ nullptr },
				.waitSemaphoreCount{ 0U },
				.pWaitSemaphores{ nullptr },
				.pWaitDstStageMask{ 0U /* VK_PIPELINE_STAGE_TRANSFER_BIT */},
				.commandBufferCount{ 1U },
				.pCommandBuffers{ &transferCommandBuffer },
				.signalSemaphoreCount{ 0U },
				.pSignalSemaphores{ nullptr },
			};

			constexpr VkFenceCreateInfo fenceCreateInfo{
				.sType{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
			};

			VkFence executionHasFinishedFence;
			VK_CHECK(vkCreateFence(
				s_LogicalDevice,
				&fenceCreateInfo,
				s_Allocator,
				&executionHasFinishedFence));

			VK_CHECK(vkQueueSubmit(
				s_Queues.Transfer,
				1U,
				&submitInfo,
				executionHasFinishedFence));

			VK_CHECK(vkWaitForFences(
				s_LogicalDevice,
				1U,
				&executionHasFinishedFence,
				VK_FALSE,
				std::numeric_limits<uint64_t>::max()));

			vkFreeCommandBuffers(
				s_LogicalDevice,
				s_CommandPools.Transfer,
				1U,
				&transferCommandBuffer);

			vkDestroyFence(
				s_LogicalDevice,
				executionHasFinishedFence,
				s_Allocator);
		}

		[[nodiscard]] static CIN_FORCE_INLINE bool PresentAndGraphicsFamiliesShared()
		{
			return s_QueueFamilies.Graphics == s_QueueFamilies.Present;
		}

		[[nodiscard]] static CIN_FORCE_INLINE bool PresentAndGraphicsQueuesCanBeSeparate()
		{
			return 
				s_QueueFamilies.Graphics != s_QueueFamilies.Present ||
				s_QueueFamilies.GraphicsQueueCount > 2;
		}

		[[nodiscard]] static CIN_FORCE_INLINE bool PresentAndGraphicsQueuesShared()
		{
			return s_Queues.Graphics == s_Queues.Present;
		}

		static CIN_FORCE_INLINE VkInstance GetInstance()
		{
			return s_Instance;
		}

		static CIN_FORCE_INLINE Surface* GetSurface()
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
#ifdef CIN_DEBUG
		static CIN_FORCE_INLINE VkDebugUtilsMessengerEXT GetDebugUtilitiesCallback()
		{
			return s_DebugUtilitiesMessenger;
		}

		static CIN_FORCE_INLINE consteval VkAllocationCallbacks* GetAllocator()
		{
			return s_Allocator;
		}
#else
		static consteval VkDebugReportCallbackEXT GetDebugReportCallback()
		{
			return nullptr;
		}

		static consteval VkAllocationCallbacks* GetAllocator()
		{
			return nullptr;
		}
#endif
		static CIN_FORCE_INLINE int32_t GetQueueFamily(const EQueueFamily queueFamily)
		{
			switch (queueFamily)
			{
				case EQueueFamily::Graphics:	return s_QueueFamilies.Graphics;
				case EQueueFamily::Compute:		return s_QueueFamilies.Compute;
				case EQueueFamily::Transfer:	return s_QueueFamilies.Transfer;
				case EQueueFamily::Present:		return s_QueueFamilies.Present;
			}

			CIN_ASSERT(false, "Unknown queue family");
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
		static inline VkPhysicalDevice s_PhysicalDevice{ VK_NULL_HANDLE };
		static inline VkDevice s_LogicalDevice{ VK_NULL_HANDLE };
		static inline Surface* s_Surface;
		static inline Swapchain* s_Swapchain;
#ifdef CIN_DEBUG
		static inline VkDebugUtilsMessengerEXT s_DebugUtilitiesMessenger{ VK_NULL_HANDLE };
#endif
		static inline constexpr VkAllocationCallbacks* s_Allocator{ VK_NULL_HANDLE };

		struct QueueFamilies final
		{
			enum : int32_t
			{
				Absent = -1,
			};

			/* Queue family handles are stored as 32 bit signed integers 
			instead of 32 bit unsigned integers to ease checking */
			int32_t Graphics;
			int32_t Compute;
			int32_t Transfer;
			int32_t Present;

			uint32_t GraphicsQueueCount;
			uint32_t ComputeQueueCount;
			uint32_t TransferQueueCount;
			uint32_t PresentQueueCount;

			constexpr QueueFamilies() noexcept
				:
				Graphics(Absent),
				Compute(Absent),
				Transfer(Absent),
				Present(Absent),

				GraphicsQueueCount(0U),
				ComputeQueueCount(0U),
				TransferQueueCount(0U),
				PresentQueueCount(0U)
			{}

			constexpr ~QueueFamilies() noexcept = default;
		} static inline s_QueueFamilies{};

		struct Queues final
		{
			VkQueue Graphics;
			VkQueue Compute;
			VkQueue Transfer;
			VkQueue Present;

			constexpr Queues() noexcept
				:
				Graphics(VK_NULL_HANDLE),
				Compute(VK_NULL_HANDLE),
				Transfer(VK_NULL_HANDLE),
				Present(VK_NULL_HANDLE)
			{}

			constexpr ~Queues() noexcept = default;
		} static inline s_Queues{};

		struct CommandPools final
		{
			VkCommandPool Graphics;
			VkCommandPool Compute;
			VkCommandPool Transfer;
			VkCommandPool Present;

			constexpr CommandPools() noexcept
				:
				Graphics(VK_NULL_HANDLE),
				Compute(VK_NULL_HANDLE),
				Transfer(VK_NULL_HANDLE),
				Present(VK_NULL_HANDLE)
			{}

			constexpr ~CommandPools() noexcept = default;
		} static inline s_CommandPools{};
	};
}