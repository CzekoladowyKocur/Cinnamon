#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

namespace Cinnamon {
	Framebuffer::Framebuffer(
		const STL::Unique<VulkanAllocator>& allocator,
		FramebufferSpecification&& framebufferSpecification) noexcept
		:
		m_Allocator(allocator),
		m_Swapchain(nullptr),
		m_Handle(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE),
		m_Sampler(VK_NULL_HANDLE),
		m_Specification(std::move(framebufferSpecification)),
		m_Attachments()
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
		m_Attachments()
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
		m_Attachments()
	{
		Invalidate(m_Specification.Width, m_Specification.Height);
	}

	Framebuffer::~Framebuffer() noexcept
	{
		Cleanup();
	}

	void Framebuffer::Invalidate(const uint32_t width, const uint32_t height)
	{
		if (m_Swapchain)
			return;
		
		m_Specification.Width = width;
		m_Specification.Height = height;

		Cleanup();

		const VkExtent3D framebufferExtent
		{
			.width{ m_Specification.Width },
			.height{ m_Specification.Height },
			.depth{ 1 }
		};

		const auto& attachmentSpecifications{ m_Specification.AttachmentSpecifications };
		m_Attachments.resize(attachmentSpecifications.size());
		for (size_t i{ 0U }; i < attachmentSpecifications.size(); ++i)
		{
			const VkImageCreateInfo imageCreateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.imageType{ VK_IMAGE_TYPE_2D },
				.format{ static_cast<VkFormat>(attachmentSpecifications[i].Format)},
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
				m_Attachments[i].Allocation,
				m_Attachments[i].Image));

			const VkImageViewCreateInfo attachmentViewCreateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.image{ m_Attachments[i].Image },
				.viewType{ VK_IMAGE_VIEW_TYPE_2D },
				.format{ static_cast<VkFormat>(attachmentSpecifications[i].Format) },
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
				&m_Attachments[i].ImageView));
			
			m_Attachments[i].Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			m_Attachments[i].Format = static_cast<VkFormat>(attachmentSpecifications[i].Format);
			m_Attachments[i].ClearColor = VkClearColorValue
			{
				attachmentSpecifications[i].ClearColor.r,
				attachmentSpecifications[i].ClearColor.g,
				attachmentSpecifications[i].ClearColor.b,
				attachmentSpecifications[i].ClearColor.a
			};

			const VkImageSubresourceRange& subresourceRange = attachmentViewCreateInfo.subresourceRange;
			m_Allocator->GetDevice()->PerformSingleSubmitGraphicsOperation([this, i, subresourceRange](const VkCommandBuffer commandBuffer)
			{
				InsertImageMemoryBarrier(
					commandBuffer,
					m_Attachments[i].Image,
					0U,
					0U,
					VK_IMAGE_LAYOUT_UNDEFINED,
					m_Attachments[i].Layout,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					subresourceRange);
			});
			
		}		
	
		STL::Vector<VkAttachmentDescription> attachmentDescriptions;
		STL::Vector<VkAttachmentReference> colorAttachmentReferences;
		STL::Vector<VkImageView> attachmentViews;

		for (size_t i{ 0U }; i < m_Attachments.size(); ++i)
		{
			attachmentDescriptions.emplace_back
			(
				VkAttachmentDescription
				{
					.flags			{ 0U																									},
					.format			{ m_Attachments[i].Format																				},
					.samples		{ VK_SAMPLE_COUNT_1_BIT																					},
					.loadOp			{ m_Specification.ClearOnLoad ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD				},
					.storeOp		{ VK_ATTACHMENT_STORE_OP_STORE																			},
					.stencilLoadOp	{ VK_ATTACHMENT_LOAD_OP_DONT_CARE																		},
					.stencilStoreOp	{ VK_ATTACHMENT_STORE_OP_DONT_CARE																		},
					.initialLayout	{ m_Specification.ClearOnLoad ? VK_IMAGE_LAYOUT_UNDEFINED	: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	},
					.finalLayout	{ m_Swapchain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR				: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL	}
				}
			);

			colorAttachmentReferences.emplace_back(VkAttachmentReference{ static_cast<uint32_t>(i), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			attachmentViews.emplace_back(m_Attachments[i].ImageView);
		}
		
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

		if (not m_Swapchain)
		{
			const VkSamplerCreateInfo samplerCreateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.magFilter{ VK_FILTER_LINEAR },
				.minFilter{ VK_FILTER_LINEAR },
				.mipmapMode{ VK_SAMPLER_MIPMAP_MODE_LINEAR },
				.addressModeU{ VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
				.addressModeV{ VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
				.addressModeW{ VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
				.mipLodBias{ 0.0f },
				.anisotropyEnable{ VK_FALSE },
				.maxAnisotropy{ 1.0f },
				.compareEnable{ VK_FALSE },
				.compareOp{ VK_COMPARE_OP_NEVER },
				.minLod{ 0.0f },
				.maxLod{ 100.0f },
				.borderColor{ VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE },
				.unnormalizedCoordinates{ VK_FALSE }
			};

			VK_CHECK(vkCreateSampler(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				&samplerCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Sampler));
		}		
	}

	void Framebuffer::Cleanup()
	{
		if (m_Swapchain)
			return;

		for (FramebufferAttachment& attachment : m_Attachments)
		{
			[[likely]]
			if(attachment.ImageView)
				vkDestroyImageView(
					m_Allocator->GetDevice()->GetLogicalDevice(),
					attachment.ImageView,
					GraphicsContext::GetAllocator());

			[[likely]]
			if (attachment.Image)
				m_Allocator->DestroyImage(attachment.Image, attachment.Allocation);

			attachment.Image = VK_NULL_HANDLE;
			attachment.ImageView = VK_NULL_HANDLE;
			attachment.Allocation = VK_NULL_HANDLE;
			attachment.Layout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.Format = VK_FORMAT_UNDEFINED;
		}

		[[likely]]
		if (m_RenderPass)
		{
			vkDestroyRenderPass(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_RenderPass,
				GraphicsContext::GetAllocator());

			m_RenderPass = VK_NULL_HANDLE;
		}

		[[likely]]
		if (m_Handle)
		{
			vkDestroyFramebuffer(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_Handle,
				GraphicsContext::GetAllocator());

			m_Handle = VK_NULL_HANDLE;
		}

		[[likely]]
		if (m_Sampler)
		{
			vkDestroySampler(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				m_Sampler,
				GraphicsContext::GetAllocator());

			m_Sampler = VK_NULL_HANDLE;
		}

		m_Attachments.clear();
	}

	const bool Framebuffer::IsClearedOnLoad() const
	{
		if (m_Swapchain)
			return true;

		return m_Specification.ClearOnLoad;
	}

	uint32_t Framebuffer::GetWidth() const
	{
		if (m_Swapchain)
			return m_Swapchain->GetExtent().width;

		return m_Specification.Width;
	}

	uint32_t Framebuffer::GetHeight() const
	{
		if (m_Swapchain)
			return m_Swapchain->GetExtent().height;

		return m_Specification.Height;
	}

	std::pair<uint32_t, uint32_t> Framebuffer::GetSize() const
	{
		if (m_Swapchain)
			return std::pair<uint32_t, uint32_t>{ m_Swapchain->GetExtent().width, m_Swapchain->GetExtent().height };

		return std::pair<uint32_t, uint32_t>{ m_Specification.Width, m_Specification.Height };
	}

	VkFramebuffer Framebuffer::GetHandle() const
	{
		if (m_Swapchain)
			return m_Swapchain->GetCurrentFramebuffer();

		CIN_ASSERT(m_Handle);
		return m_Handle;
	}

	VkRenderPass Framebuffer::GetRenderPass() const
	{
		if (m_Swapchain)
			return m_Swapchain->GetRenderPass();

		CIN_ASSERT(m_RenderPass);
		return m_RenderPass;
	}

	VkSampler Framebuffer::GetSampler() const
	{
		CIN_ASSERT(not m_Swapchain and m_Sampler);
		return m_Sampler;
	}

	uint32_t Framebuffer::GetColorAttachmentCount() const
	{
		if (m_Swapchain)
			return 1U;

		return static_cast<uint32_t>(m_Attachments.size());
	}

	VkImage Framebuffer::GetColorAttachment(const size_t attachmentIndex) const
	{
		CIN_ASSERT(m_Attachments.size() > attachmentIndex);
		CIN_ASSERT(m_Attachments[attachmentIndex].Image);
		CIN_ASSERT(not m_Swapchain);

		return m_Attachments[attachmentIndex].Image;
	}

	VkImageView Framebuffer::GetColorAttachmentView(const size_t attachmentIndex) const
	{
		CIN_ASSERT(m_Attachments.size() > attachmentIndex);
		CIN_ASSERT(m_Attachments[attachmentIndex].ImageView);
		CIN_ASSERT(not m_Swapchain);

		return m_Attachments[attachmentIndex].ImageView;
	}

	VkFormat Framebuffer::GetColorAttachmentFormat(const size_t attachmentIndex) const
	{
		if (m_Swapchain)
		{
			CIN_ASSERT(attachmentIndex == 0U);
			return m_Swapchain->GetFormat();
		}

		CIN_ASSERT(m_Attachments.size() > attachmentIndex);
		CIN_ASSERT(m_Attachments[attachmentIndex].Format != VK_FORMAT_UNDEFINED);

		return m_Attachments[attachmentIndex].Format;
	}

	VkImageLayout Framebuffer::GetColorAttachmentLayout(const size_t attachmentIndex) const
	{
		CIN_ASSERT(m_Attachments.size() > attachmentIndex);
		CIN_ASSERT(m_Attachments[attachmentIndex].Layout != VK_IMAGE_LAYOUT_UNDEFINED);
		CIN_ASSERT(not m_Swapchain);

		return m_Attachments[attachmentIndex].Layout;
	}

	VkClearColorValue Framebuffer::GetColorAttachmentClearValue(const size_t attachmentIndex) const
	{
		if (m_Swapchain)
			return m_Swapchain->GetClearValue().color;

		CIN_ASSERT(m_Attachments.size() > attachmentIndex);
		
		return m_Attachments[attachmentIndex].ClearColor;
	}
}