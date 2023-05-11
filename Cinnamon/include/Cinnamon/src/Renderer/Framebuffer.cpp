#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"

namespace Cinnamon {
	Framebuffer::Framebuffer(
		const STL::Unique<VulkanAllocator>& allocator,
		FramebufferSpecification&& framebufferSpecification) noexcept
		:
		m_Allocator(allocator),
		m_Swapchain(nullptr),
		m_Handle(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE),
		m_Specification(std::move(framebufferSpecification)),
		m_Attachment(VK_NULL_HANDLE),
		m_AttachmentView(VK_NULL_HANDLE),
		m_AttachmentAllocation(VK_NULL_HANDLE)
	{
		Invalidate(m_Specification.Width, m_Specification.Height);
	}

	Framebuffer::Framebuffer(
		const STL::Unique<VulkanAllocator>& allocator,
		FramebufferSpecification&& framebufferSpecification,
		const STL::Unique<Swapchain>& swapchain) noexcept
		:
		m_Allocator(allocator),
		m_Swapchain(swapchain.get()),
		m_Handle(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE),
		m_Specification(std::move(framebufferSpecification)),
		m_Attachment(VK_NULL_HANDLE),
		m_AttachmentView(VK_NULL_HANDLE),
		m_AttachmentAllocation(VK_NULL_HANDLE)
	{
		Invalidate(m_Specification.Width, m_Specification.Height);
	}

	Framebuffer::Framebuffer(
		const STL::Unique<VulkanAllocator>& allocator,
		const FramebufferSpecification& framebufferSpecification) noexcept
		:
		m_Allocator(allocator),
		m_Swapchain(nullptr),
		m_Handle(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE),
		m_Specification(framebufferSpecification),
		m_Attachment(VK_NULL_HANDLE),
		m_AttachmentView(VK_NULL_HANDLE),
		m_AttachmentAllocation(VK_NULL_HANDLE)
	{
		Invalidate(m_Specification.Width, m_Specification.Height);
	}

	Framebuffer::~Framebuffer() noexcept
	{
		Cleanup();
	}

	void Framebuffer::Invalidate(const uint32_t width, const uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		
		if (m_Swapchain)
		{
			m_RenderPass = m_Swapchain->GetRenderPass();
			return;
		}

		Cleanup();

		const VkExtent3D framebufferExtent
		{
			.width{ m_Specification.Width },
			.height{ m_Specification.Height },
			.depth{ 1 }
		};

		const VkImageCreateInfo imageCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.imageType{ VK_IMAGE_TYPE_2D },
			.format{ static_cast<VkFormat>(m_Specification.AttachmentFormat) },
			.extent{ framebufferExtent },
			.mipLevels{ 1U },
			.arrayLayers{ 1U },
			.samples{ VK_SAMPLE_COUNT_1_BIT },
			.tiling{ VK_IMAGE_TILING_OPTIMAL },
			.usage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT },
			.sharingMode{ VK_SHARING_MODE_EXCLUSIVE },
			.queueFamilyIndexCount{ VK_QUEUE_FAMILY_IGNORED  },
			.pQueueFamilyIndices{ VK_NULL_HANDLE },
			.initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED },
		};

		CIN_VERIFY(m_Allocator->AllocateImage(
			imageCreateInfo, 
			VMA_MEMORY_USAGE_GPU_ONLY, 
			m_AttachmentAllocation, 
			m_Attachment));
		
		const VkImageViewCreateInfo attachmentViewCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.image{ m_Attachment },
			.viewType{ VK_IMAGE_VIEW_TYPE_2D },
			.format{ static_cast<VkFormat>(m_Specification.AttachmentFormat) },
			.components
			{
				.r{ VK_COMPONENT_SWIZZLE_R },
				.g{ VK_COMPONENT_SWIZZLE_G },
				.b{ VK_COMPONENT_SWIZZLE_B },
				.a{ VK_COMPONENT_SWIZZLE_A }
			},
			.subresourceRange
			{
				.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
				.baseMipLevel{ 0U },
				.levelCount{ 1U },
				.baseArrayLayer{ 0U },
				.layerCount{ 1U },
			}
		};

		VK_CHECK(vkCreateImageView(
			m_Allocator->GetDevice()->GetLogicalDevice(),
			&attachmentViewCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_AttachmentView));

		m_AttachmentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		const VkImageSubresourceRange& subresourceRange = attachmentViewCreateInfo.subresourceRange;
		m_Allocator->GetDevice()->PerformSingleSubmitGraphicsOperation([this, subresourceRange](const VkCommandBuffer commandBuffer)
		{
			InsertImageMemoryBarrier(
				commandBuffer,
				m_Attachment,
				0U,
				0U,
				VK_IMAGE_LAYOUT_UNDEFINED,
				m_AttachmentLayout,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				subresourceRange);
		});

		STL::Vector<VkImageView> attachmentViews;
		attachmentViews.push_back(m_AttachmentView);

		STL::Vector<VkAttachmentDescription> attachmentDescriptions;
		STL::Vector<VkAttachmentReference> colorAttachmentReferences;

		VkAttachmentDescription& attachmentDescription{ attachmentDescriptions.emplace_back(VkAttachmentDescription{}) };
		attachmentDescription.format = static_cast<VkFormat>(m_Specification.AttachmentFormat);
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout = m_Swapchain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescription.flags = 0U;

		colorAttachmentReferences.emplace_back(VkAttachmentReference{ 0U, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		
		VkSubpassDescription subpassDescription;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
		subpassDescription.pColorAttachments = colorAttachmentReferences.data();
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.flags = 0U;

		STL::Vector<VkSubpassDependency> subpassDependencies;
		VkSubpassDependency& dependency = subpassDependencies.emplace_back();

		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.subpassCount = 1U;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
		renderPassCreateInfo.pDependencies = subpassDependencies.data();
		renderPassCreateInfo.flags = 0U;
		renderPassCreateInfo.pNext = nullptr;

		/* Check if render pass is compatible. . . */
		VK_CHECK(vkCreateRenderPass(
			m_Allocator->GetDevice()->GetLogicalDevice(),
			&renderPassCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_RenderPass));

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
		framebufferCreateInfo.pAttachments = attachmentViews.data();
		framebufferCreateInfo.width = m_Specification.Width;
		framebufferCreateInfo.height = m_Specification.Height;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreateFramebuffer(
			m_Allocator->GetDevice()->GetLogicalDevice(),
			&framebufferCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));
	}

	void Framebuffer::Cleanup()
	{
		if (m_Swapchain)
			return;
		
		if (m_AttachmentView)
		{
			vkDestroyImageView(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_AttachmentView,
				GraphicsContext::GetAllocator());
		}

		if (m_Attachment)
			m_Allocator->DestroyImage(m_Attachment, m_AttachmentAllocation);

		if (m_RenderPass)
		{
			vkDestroyRenderPass(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_RenderPass,
				GraphicsContext::GetAllocator());
		}

		if (m_Handle)
		{
			vkDestroyFramebuffer(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_Handle,
				GraphicsContext::GetAllocator());
		}
	}

	std::pair<uint32_t, uint32_t> Framebuffer::GetSize() const
	{
		return std::pair<uint32_t, uint32_t>{ m_Specification.Width, m_Specification.Height };
	}

	VkFramebuffer Framebuffer::GetHandle() const
	{
		CIN_ASSERT(m_Handle);
		return m_Handle;
	}

	VkRenderPass Framebuffer::GetRenderPass() const
	{
		CIN_ASSERT(m_RenderPass);
		return m_RenderPass;
	}

	VkImage Framebuffer::GetColorAttachment() const
	{
		CIN_ASSERT(m_Attachment);
		return m_Attachment;
	}

	VkImageView Framebuffer::GetColorAttachmentView() const
	{
		CIN_ASSERT(m_AttachmentView);
		return m_AttachmentView;
	}

	VkImageLayout Framebuffer::GetColorAttachmentLayout() const
	{
		return m_AttachmentLayout;
	}
}