#pragma once
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/Image.hpp"

namespace Cinnamon {
	class Swapchain;

	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Samples;
		EImageFormat AttachmentFormat;
	};

	class Framebuffer final
	{
	private:
		NON_COPYABLE(Framebuffer)
	public:
		explicit Framebuffer(
			const STL::Unique<VulkanAllocator>& allocator,
			FramebufferSpecification&& framebufferSpecification) noexcept;

		explicit Framebuffer(
			const STL::Unique<VulkanAllocator>& allocator,
			FramebufferSpecification&& framebufferSpecification,
			const STL::Unique<Swapchain>& swapchain) noexcept;
		
		explicit Framebuffer(
			const STL::Unique<VulkanAllocator>& allocator,
			const FramebufferSpecification& framebufferSpecification) noexcept;
		
		~Framebuffer() noexcept;

		void Invalidate(const uint32_t width, const uint32_t height);
		void Cleanup();

		[[nodiscard]] std::pair<uint32_t, uint32_t> 
			GetSize() const;
		
		[[nodiscard]] VkFramebuffer 
			GetHandle() const;
		
		[[nodiscard]] VkRenderPass 
			GetRenderPass() const;
		
		[[nodiscard]] VkImage
			GetColorAttachment() const;

		[[nodiscard]] VkImageView
			GetColorAttachmentView() const;

		[[nodiscard]] VkImageLayout 
			GetColorAttachmentLayout() const;
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;
		Swapchain* m_Swapchain;

		VkFramebuffer m_Handle;
		VkRenderPass m_RenderPass;
		FramebufferSpecification m_Specification;

		/* Currently only one image supported */
		VkImage m_Attachment;
		VkImageView m_AttachmentView;
		VkImageLayout m_AttachmentLayout;

		VmaAllocation m_AttachmentAllocation;

		friend class Renderer;
		friend class Pipeline;
	};
}