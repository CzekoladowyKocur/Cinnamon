#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.h"

namespace Cinnamon {
	class Surface;
	class Device;
}

namespace Cinnamon {
	class Swapchain final
	{
	private:
		NON_COPYABLE(Swapchain)
	public:
		explicit Swapchain(
			const STL::Unique<Surface>& surface,
			const STL::Unique<Device>& device,
			const uint32_t width,
			const uint32_t height);

		~Swapchain();

		void Create(
			const uint32_t width,
			const uint32_t height);

		void Recreate(
			const uint32_t width,
			const uint32_t height);

		void Cleanup();

		void SetClearColor(
			const float r,
			const float g,
			const float b,
			const float a);

		void AcquireNextSwapchainImage();
		void PresentSwapchainImage();
		void RecordCommands(const std::function<void()> recordFunction);

		uint32_t GetImageCount() const;
		uint32_t GetImageIndex() const;
		uint32_t GetFrameIndex() const;
		VkExtent2D GetExtent() const;
		VkRenderPass GetRenderPass() const;
		VkCommandBuffer GetCurrentCommandBuffer() const;
		VkFramebuffer GetCurrentFramebuffer() const;
	private:
		const STL::Unique<Surface>&		m_Surface;
		const STL::Unique<Device>&		m_Device;

		VkSwapchainKHR m_Handle;
		VkSwapchainKHR m_CachedSwapchain;
		VkRenderPass m_RenderPass;
		VkCommandPool m_CommandPool;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
		VkExtent2D m_Extent;
		VkClearValue m_ClearColor;

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

		std::function<void()> m_RecordFunction;
	};
}