#pragma once
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/Image.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class Swapchain;

	struct FramebufferAttachmentSpecification
	{
		EImageFormat		Format;
		CinMath::Vector4	ClearColor;
	};

	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Samples;
		bool ClearOnLoad;

		STL::Vector<FramebufferAttachmentSpecification> AttachmentSpecifications;
	};

	class Framebuffer final
	{
	private:
		struct FramebufferAttachment
		{
			VkImage				Image{ VK_NULL_HANDLE };
			VkImageView			ImageView{ VK_NULL_HANDLE };
			VmaAllocation		Allocation{ VK_NULL_HANDLE };
			VkFormat			Format{ VK_FORMAT_UNDEFINED };
			VkImageLayout		Layout{ VK_IMAGE_LAYOUT_UNDEFINED };
			VkClearColorValue	ClearColor;
		};

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

		[[nodiscard]] bool
			IsClearedOnLoad() const;

		[[nodiscard]] uint32_t
			GetWidth() const;

		[[nodiscard]] uint32_t
			GetHeight() const;

		[[nodiscard]] std::pair<uint32_t, uint32_t> 
			GetSize() const;
		
		[[nodiscard]] VkFramebuffer 
			GetHandle() const;
		
		[[nodiscard]] VkRenderPass 
			GetRenderPass() const;

		[[nodiscard]] VkSampler
			GetSampler() const;

		[[nodiscard]] uint32_t
			GetColorAttachmentCount() const;
		
		[[nodiscard]] VkImage
			GetColorAttachment(const size_t attachmentIndex) const;

		[[nodiscard]] VkImageView
			GetColorAttachmentView(const size_t attachmentIndex) const;

		[[nodiscard]] VkFormat
			GetColorAttachmentFormat(const size_t attachmentIndex) const;

		[[nodiscard]] VkImageLayout 
			GetColorAttachmentLayout(const size_t attachmentIndex) const;

		[[nodiscard]] VkClearColorValue
			GetColorAttachmentClearValue(const size_t attachmentIndex) const;
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;
		Swapchain* m_Swapchain;

		VkFramebuffer m_Handle;
		VkRenderPass m_RenderPass;
		VkSampler m_Sampler;
		FramebufferSpecification m_Specification;

		/* Currently only one image supported */
		STL::Vector<FramebufferAttachment> m_Attachments;
		friend class Renderer;
	};
}