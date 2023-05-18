#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	class Window;
	class Device;
}

namespace Cinnamon {
	using SwapchainCommandRecordCallback = std::function<void(
		const VkCommandBuffer,
		const VkFramebuffer,
		const VkRenderPass,
		const VkExtent2D)>;

	class Swapchain final
	{
	private:
		NON_COPYABLE(Swapchain)
	public:
		explicit Swapchain(
			const STL::Unique<Device>& device,
			const STL::Unique<Window>& window) noexcept;

		~Swapchain() noexcept;

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
		void RecordCommands(const SwapchainCommandRecordCallback recordFunction);

		[[nodiscard]] uint32_t 
			GetImageCount() const;

		[[nodiscard]] uint32_t
			GetImageMinimalCount() const;

		[[nodiscard]] uint32_t 
			GetImageIndex() const;

		[[nodiscard]] uint32_t 
			GetFrameIndex() const;

		[[nodiscard]] VkFormat
			GetFormat() const;

		[[nodiscard]] VkExtent2D 
			GetExtent() const;

		[[nodiscard]] VkClearValue
			GetClearValue() const;

		[[nodiscard]] VkRenderPass 
			GetRenderPass() const;

		[[nodiscard]] VkFramebuffer 
			GetCurrentFramebuffer() const;

		[[nodiscard]] VkCommandBuffer 
			GetCommandBuffer(const uint32_t frameIndex) const;

		[[nodiscard]] VkFence
			GetWaitFence(const uint32_t frameIndex) const;
	private:
		const STL::Unique<Device>& m_Device;
		VkSurfaceKHR m_Surface;

		VkSwapchainKHR m_Handle;
		VkSwapchainKHR m_CachedSwapchain;
		VkRenderPass m_RenderPass;
		VkCommandPool m_CommandPool;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSurfaceCapabilitiesKHR m_SurfaceCapabilities;
		VkExtent2D m_Extent;
		VkClearValue m_ClearColor;
		uint32_t m_MinimalImageCount;
		bool m_SurfaceUpdated;

		STL::Vector<VkImage> m_Images;
		STL::Vector<VkImageView> m_ImageViews;
		STL::Vector<VkFramebuffer> m_Framebuffers;
		STL::Vector<VkCommandBuffer> m_CommandBuffers;

		struct {
			STL::Vector<VkSemaphore> ImageAvailable;
			STL::Vector<VkSemaphore> RenderingFinished;
		} m_Semaphores;

		struct {
			STL::Vector<VkFence> InFlightFences;
		} m_Fences;

		uint32_t m_ImageIndex;
		uint32_t m_FrameIndex;
		uint32_t m_FramesInFlight;

		SwapchainCommandRecordCallback m_RecordFunction;
	};
}