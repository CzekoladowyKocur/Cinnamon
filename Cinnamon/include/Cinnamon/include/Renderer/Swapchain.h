#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"

namespace Cinnamon {
	class Surface;
}

namespace Cinnamon {
	class Swapchain
	{
	private:
	public:
		Swapchain(const uint32_t width, const uint32_t height, const Surface* const surface) noexcept;
		~Swapchain() noexcept;

		void Create(const uint32_t width, const uint32_t height, const Surface* const surface);
		void Recreate(const uint32_t width, const uint32_t height, const Surface* const surface);

		void AcquireNextSwapchainImage();
		void PresentSwapchainImage();
	private:
		void Cleanup();
	private:
		VkSwapchainKHR m_Handle;
		VkSwapchainKHR m_CachedSwapchain;
		VkRenderPass m_RenderPass;
		VkCommandPool m_CommandPool;
		
		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
		VkExtent2D m_Extent;

		STL::Vector<VkImage> m_Images;
		STL::Vector<VkImageView> m_ImageViews;
		STL::Vector<VkFramebuffer> m_Framebuffers;
		STL::Vector<VkCommandBuffer> m_CommandBuffers;

		struct {
			std::vector<VkSemaphore> ImageAvailable;
			std::vector<VkSemaphore> RenderingFinished;
		} m_Semaphores;

		struct {
			std::vector<VkFence> InFlightFences;
		} m_Fences;

		uint32_t m_ImageIndex;
		uint32_t m_FrameIndex;
		uint32_t m_FramesInFlight;
	};
}