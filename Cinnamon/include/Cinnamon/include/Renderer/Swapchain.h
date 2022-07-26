#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"

namespace Cinnamon {
	class Swapchain
	{
	private:
	public:
		Swapchain(const uint32_t width, const uint32_t height, VkSurfaceKHR surface) noexcept;
		~Swapchain() noexcept;

		void AcquireNextSwapchainImage();
		void PresentSwapchainImage();

		void Create(const uint32_t width, const uint32_t height, VkSurfaceKHR surface);
	private:
		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
		VkExtent2D m_Extent;

		VkSwapchainKHR m_Handle;
		VkRenderPass m_RenderPass;

		STL::Vector<VkImage> m_Images;
		STL::Vector<VkImageView> m_ImageViews;
		STL::Vector<VkFramebuffer> m_Framebuffers;

		VkCommandPool m_CommandPool;
		STL::Vector<VkCommandBuffer> m_CommandBuffers;

		struct {
			std::vector<VkSemaphore> ImageAvailable;
			std::vector<VkSemaphore> RenderingFinished;
		} m_Semaphores;

		struct {
			std::vector<VkFence> InFlightFences;
		} m_Fences;

		uint32_t m_ImageIndex;
	};
}