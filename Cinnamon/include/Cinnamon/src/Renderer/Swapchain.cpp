#include "Cinnamon/include/Renderer/Swapchain.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"
#include "Cinnamon/include/Renderer/Surface.h"

namespace Cinnamon {
	Swapchain::Swapchain(const uint32_t width, const uint32_t height, VkSurfaceKHR surface) noexcept
		:
		m_Handle(VK_NULL_HANDLE),
		m_CachedSwapchain(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE),
		m_CommandPool(VK_NULL_HANDLE),
		m_SurfaceFormat({ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }),
		m_PresentMode(VK_PRESENT_MODE_FIFO_KHR), /* The only one guaranteed by specification */
		m_SurfaceCapabilities({}),
		m_Extent({ width, height }),
		m_Images({}),
		m_ImageViews({}),
		m_Framebuffers({}),
		m_Semaphores({}),
		m_Fences({}),
		m_ImageIndex(0U),
		m_FrameIndex(0U),
		m_FramesInFlight(2U)
	{
		const VkCommandPoolCreateInfo commandPoolCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT },
			.queueFamilyIndex{ static_cast<uint32_t>(GraphicsContext::GetQueueFamily(GraphicsContext::EQueueFamily::Graphics)) },
		};

		VK_CHECK(vkCreateCommandPool(
			GraphicsContext::GetDevice(),
			&commandPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_CommandPool));

		Create(width, height, surface);
	}

	Swapchain::~Swapchain() noexcept
	{
		VK_CHECK(vkDeviceWaitIdle(
			GraphicsContext::GetDevice()));

		Cleanup();
		vkDestroyCommandPool(
			GraphicsContext::GetDevice(),
			m_CommandPool,
			GraphicsContext::GetAllocator());
		
		vkDestroySwapchainKHR(
			GraphicsContext::GetDevice(),
			m_Handle,
			GraphicsContext::GetAllocator());
	}

	void Swapchain::Create(const uint32_t width, const uint32_t height, VkSurfaceKHR surface)
	{
		m_ImageIndex = 0U;
		m_FrameIndex = 0U;
		CIN_ASSERT(width >= 1U && width <= 15360U, "Invalid swapchain width");
		CIN_ASSERT(height >= 1U && height <= 8640U, "Invalid swapchain height");
		CIN_TRACE("Recreating swapchain: {0}, {1}", width, height);

		/* Pick surface format */
		{
			uint32_t availableSurfaceFormatCount{ 0U };
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availableSurfaceFormatCount,
				nullptr));

			CIN_ASSERT(availableSurfaceFormatCount > 0U, "No available surface format!");
			STL::Vector<VkSurfaceFormatKHR> availableSurfaceFormats(availableSurfaceFormatCount);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availableSurfaceFormatCount,
				&availableSurfaceFormats[0U]));

			bool found{ false };
			for (const VkSurfaceFormatKHR availableFormat : availableSurfaceFormats)
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					found = true;
					m_SurfaceFormat = availableFormat;
					break;
				}

			if (!found)
				m_SurfaceFormat = availableSurfaceFormats[0U];
		}

		/* Pick present mode */
		{
			uint32_t availablePresentModeCount{ 0U };
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availablePresentModeCount,
				nullptr));

			CIN_ASSERT(availablePresentModeCount > 0U, "No available present mode");
			STL::Vector<VkPresentModeKHR> availablePresentModes(availablePresentModeCount);
			
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availablePresentModeCount,
				&availablePresentModes[0]));

			bool found{ false };
			for (const VkPresentModeKHR availablePresentMode : availablePresentModes)
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					m_PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					found = true;
					break;
				}

			if (!found)
				m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		}

		/* Get surface capabilities */
		{
			VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&m_SurfaceCapabilities));
		}

		/* Adjust swapchain extent if needed */
		{
			const auto& capabilities{ m_SurfaceCapabilities };
			if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max() && capabilities.currentExtent.height != std::numeric_limits<std::uint32_t>::max())
				m_Extent = capabilities.currentExtent;
			else
			{
				VkExtent2D extent{
					.width{ width },
					.height{ height },
				};

				extent.width = CIN_CLAMP(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.height = CIN_CLAMP(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				m_Extent = extent;
			}
		}

		uint32_t imageCount{ m_SurfaceCapabilities.minImageCount + 1U };
		if (m_SurfaceCapabilities.maxImageCount > 0U && imageCount > m_SurfaceCapabilities.maxImageCount)
			imageCount = m_SurfaceCapabilities.maxImageCount;

		const bool queuesFamiliesShared{ GraphicsContext::PresentAndGraphicsFamiliesShared() };
		const STL::Array<uint32_t, 2> queueFamilyIndices
		{
			static_cast<uint32_t>(GraphicsContext::GetQueueFamily(GraphicsContext::EQueueFamily::Graphics)), 
			static_cast<uint32_t>(GraphicsContext::GetQueueFamily(GraphicsContext::EQueueFamily::Present)), 
		};

		const VkSwapchainCreateInfoKHR swapchainCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR },
			.pNext{ nullptr },
			.flags{ 0U },
			.surface{ surface },
			.minImageCount{ imageCount },
			.imageFormat{ m_SurfaceFormat.format },
			.imageColorSpace{ m_SurfaceFormat.colorSpace },
			.imageExtent{ m_Extent },
			.imageArrayLayers{ 1U },
			.imageUsage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT },
			.imageSharingMode{ VK_SHARING_MODE_EXCLUSIVE },
			.queueFamilyIndexCount{ queuesFamiliesShared ? VK_QUEUE_FAMILY_IGNORED : static_cast<uint32_t>(queueFamilyIndices.size()) },
			.pQueueFamilyIndices{ queuesFamiliesShared ? nullptr : &queueFamilyIndices[0U] },
			.preTransform{ m_SurfaceCapabilities.currentTransform },
			.compositeAlpha{ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR },
			.presentMode{ m_PresentMode },
			.clipped{ VK_TRUE },
			.oldSwapchain{ m_CachedSwapchain },
		};

		VK_CHECK(vkCreateSwapchainKHR(
			GraphicsContext::GetDevice(),
			&swapchainCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));

		/* Main color attachment */
		const VkAttachmentDescription colorAttachmentDescription{
			.flags{ 0U },
			.format{ m_SurfaceFormat.format },
			.samples{ VK_SAMPLE_COUNT_1_BIT },
			.loadOp{ VK_ATTACHMENT_LOAD_OP_CLEAR }, /* Clear previous frame before drawing to it*/
			.storeOp{ VK_ATTACHMENT_STORE_OP_STORE }, /* Store for present */
			.stencilLoadOp{ VK_ATTACHMENT_LOAD_OP_DONT_CARE },
			.stencilStoreOp{ VK_ATTACHMENT_STORE_OP_DONT_CARE },
			.initialLayout{ VK_IMAGE_LAYOUT_UNDEFINED }, /* Unknown initial layout of the attachment */
			.finalLayout{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }, /* Transfer the layout to swapchain presentable format*/
		};

		constexpr VkAttachmentReference colorAttachmentReference{
			.attachment{ 0U },
			.layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } /* Specifies the layout of the attachment during subpass */
		};

		const VkSubpassDescription subpassDescription{
			.flags{ 0U },
			.pipelineBindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS },
			.inputAttachmentCount{ 0U },
			.pInputAttachments{ nullptr },
			.colorAttachmentCount{ 1U },
			.pColorAttachments{ &colorAttachmentReference },
			.pResolveAttachments{ 0U },
			.pDepthStencilAttachment{ nullptr },
			.preserveAttachmentCount{ 0U },
			.pPreserveAttachments{ nullptr },
		};

		constexpr VkSubpassDependency subpassDependency{
			.srcSubpass{ VK_SUBPASS_EXTERNAL }, /* Is the subpass index of the first subpass in the dependency, or VK_SUBPASS_EXTERNAL. */
			.dstSubpass{ 0U }, /* Is the subpass index of the second subpass in the dependency, or VK_SUBPASS_EXTERNAL. */
			.srcStageMask{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
			.dstStageMask{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
			.srcAccessMask{ 0U },
			.dstAccessMask{ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT },
			.dependencyFlags{ 0U },
		};
		/*
		If srcSubpass is equal to VK_SUBPASS_EXTERNAL, the first synchronization scope includes commands that
		occur earlier in submission order than the vkCmdBeginRenderPass used to begin the render pass instance.
		Otherwise, the first set of commands includes all commands submitted as part of the subpass instance identified
		by srcSubpass and any load, store or multisample resolve operations on attachments used in
		srcSubpass. In either case, the first synchronization scope is limited to operations on the pipeline
		stages determined by the source stage mask specified by srcStageMask.
		*/

		const VkRenderPassCreateInfo renderPassCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.attachmentCount{ 1U },
			.pAttachments{ &colorAttachmentDescription },
			.subpassCount{ 1U },
			.pSubpasses{ &subpassDescription },
			.dependencyCount{ 1U },
			.pDependencies{ &subpassDependency },
		};

		VK_CHECK(vkCreateRenderPass(
			GraphicsContext::GetDevice(),
			&renderPassCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_RenderPass));

		/* Retrieve vulkan-side created swapchain images */
		VK_CHECK(vkGetSwapchainImagesKHR(
			GraphicsContext::GetDevice(),
			m_Handle,
			&imageCount,
			nullptr));

		CIN_ASSERT(imageCount > 0U, "Invalid image count");
		m_Images.resize(imageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(
			GraphicsContext::GetDevice(),
			m_Handle,
			&imageCount,
			&m_Images[0U]));

		VkImageViewCreateInfo imageViewCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.image{ nullptr },
			.viewType{ VK_IMAGE_VIEW_TYPE_2D },
			.format{ m_SurfaceFormat.format },
			.components{ 
				.r { VK_COMPONENT_SWIZZLE_R },
				.g { VK_COMPONENT_SWIZZLE_G },
				.b { VK_COMPONENT_SWIZZLE_B },
				.a { VK_COMPONENT_SWIZZLE_A },
			},
			.subresourceRange{
				.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
				.baseMipLevel{ 0U },
				.levelCount{ 1U },
				.baseArrayLayer{ 0U },
				.layerCount{ 1U },
			},
		};

		m_ImageViews.resize(imageCount);
		for (uint32_t i{ 0U }; i < imageCount; ++i)
		{
			imageViewCreateInfo.image = m_Images[i];

			VK_CHECK(vkCreateImageView(
				GraphicsContext::GetDevice(),
				&imageViewCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_ImageViews[i]));
		}

		VkFramebufferCreateInfo framebufferCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.renderPass{ m_RenderPass },
			.attachmentCount{ 1U },
			.pAttachments{ nullptr }, /* Set in loop for each image */
			.width{ m_Extent.width },
			.height{ m_Extent.height },
			.layers{ 1U },
		};

		m_Framebuffers.resize(imageCount);
		for (uint32_t i{ 0U }; i < imageCount; ++i)
		{
			framebufferCreateInfo.pAttachments = &m_ImageViews[i];

			VK_CHECK(vkCreateFramebuffer(
				GraphicsContext::GetDevice(),
				&framebufferCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Framebuffers[i]));
		}

		m_CommandBuffers.resize(imageCount);
		const VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO },
			.pNext{ nullptr },
			.commandPool{ m_CommandPool },
			.level{ VK_COMMAND_BUFFER_LEVEL_PRIMARY },
			.commandBufferCount{ imageCount },
		};

		VK_CHECK(vkAllocateCommandBuffers(
			GraphicsContext::GetDevice(),
			&commandBufferAllocateInfo,
			m_CommandBuffers.data()));

		m_Fences.InFlightFences.resize(m_FramesInFlight);
		m_Semaphores.ImageAvailable.resize(m_FramesInFlight);
		m_Semaphores.RenderingFinished.resize(m_FramesInFlight);

		constexpr VkFenceCreateInfo fenceCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_FENCE_CREATE_SIGNALED_BIT },
		};

		constexpr VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_SEMAPHORE_TYPE_BINARY },
		};

		for (uint32_t i{ 0U }; i < m_FramesInFlight; ++i)
		{
			VK_CHECK(vkCreateFence(
				GraphicsContext::GetDevice(),
				&fenceCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Fences.InFlightFences[i]));

			VK_CHECK(vkCreateSemaphore(
				GraphicsContext::GetDevice(),
				&semaphoreCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Semaphores.ImageAvailable[i]));

			VK_CHECK(vkCreateSemaphore(
				GraphicsContext::GetDevice(),
				&semaphoreCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Semaphores.RenderingFinished[i]));
		}

		CIN_TRACE("Created {0}-buffered swapchain with present and graphics queue {1}", imageCount, queuesFamiliesShared ? "shared" : "not shared");
	}

	void Swapchain::Recreate(const uint32_t width, const uint32_t height, VkSurfaceKHR surface)
	{
		VK_CHECK(vkDeviceWaitIdle(
			GraphicsContext::GetDevice()));

		/* If surface hasn't changed, cache handle */
		m_CachedSwapchain = GraphicsContext::GetSurface()->GetHandle() == surface ? m_Handle : VK_NULL_HANDLE;
		Cleanup();
		Create(width, height, surface);
		vkDestroySwapchainKHR(
			GraphicsContext::GetDevice(),
			m_CachedSwapchain,
			GraphicsContext::GetAllocator());
	}

	void Swapchain::AcquireNextSwapchainImage()
	{
		const VkFence imageInFlightFence{ m_Fences.InFlightFences[m_FrameIndex] };
		const VkSemaphore presentCompleteSemaphore{ m_Semaphores.ImageAvailable[m_FrameIndex] };

		VK_CHECK(vkWaitForFences(
			GraphicsContext::GetDevice(),
			1U,
			&imageInFlightFence,
			VK_TRUE,
			std::numeric_limits<std::uint64_t>::max()));

		VkResult result;
		do {
			result = vkAcquireNextImageKHR(
				GraphicsContext::GetDevice(),
				m_Handle,
				std::numeric_limits<std::uint64_t>::max(),
				presentCompleteSemaphore,
				VK_NULL_HANDLE, /* Fence here instead? */
				&m_ImageIndex);

			switch (result)
			{
				case VK_SUCCESS:
				{
				} break;

				case VK_ERROR_OUT_OF_DATE_KHR:
				{
					GraphicsContext::ResizeSwapchain();
				} break;

				case VK_SUBOPTIMAL_KHR:
				{
					GraphicsContext::ResizeSwapchain();
				} break;

				case VK_ERROR_SURFACE_LOST_KHR:
				{
					GraphicsContext::RecreateSurface();
					return;
				} break;

				default:
				{
					GraphicsContext::RecreateSurface();
					CIN_WARN("Unhandled acquire result: {0}", VKResultToString(result));
				} break;
			}
		} while (result != VK_SUCCESS);

		/* Unsignal the fence */
		VK_CHECK(vkResetFences(
			GraphicsContext::GetDevice(),
			1U,
			&imageInFlightFence));
	}

	void Swapchain::PresentSwapchainImage()
	{
		/* Signals images in flight */
		const VkFence imageInFlightFence{ m_Fences.InFlightFences[m_FrameIndex] };
		/* Signals whether presenting the image has finished (image is available) */
		const VkSemaphore presentCompleteSemaphore{ m_Semaphores.ImageAvailable[m_FrameIndex] };
		/* Signals whether rendering the image has finished (image was rendered to) */
		const VkSemaphore renderCompleteSemaphore{ m_Semaphores.RenderingFinished[m_FrameIndex] };
		/* Current frmae command buffer */
		const VkCommandBuffer commandBuffer{ m_CommandBuffers[m_FrameIndex] };

		/* Color attachment output */
		constexpr STL::Array<VkPipelineStageFlags, 1U> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		const VkSubmitInfo submitInfo{
			.sType{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
			.pNext{ nullptr},
			.waitSemaphoreCount{ 1U },
			.pWaitSemaphores{ &presentCompleteSemaphore },
			.pWaitDstStageMask{ &waitStages[0U] },
			.commandBufferCount{ 1U },
			.pCommandBuffers{ &commandBuffer },
			.signalSemaphoreCount{ 1U },
			.pSignalSemaphores{ &renderCompleteSemaphore },
		};

		/* Record default clear command buffers */
		{
			constexpr STL::Array<VkClearValue, 1U> clearValues{
				VkClearValue{
					.color{
						.float32{
							0.3f, 0.1f, 0.12f, 1.0f
						}
					},
				}
			};

			constexpr VkCommandBufferBeginInfo commandBufferBeginInfo{
				.sType{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO },
				.pNext{ nullptr },
				.flags{ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT },
				.pInheritanceInfo{ nullptr },
			};

			const VkRenderPassBeginInfo renderPassBeginInfo{
				.sType{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO },
				.pNext{ nullptr },
				.renderPass{ m_RenderPass },
				.framebuffer{ m_Framebuffers[m_ImageIndex] },
				.renderArea{
					.offset{
						.x{ 0 },
						.y{ 0 },
					},
					.extent{ m_Extent },
				},
				.clearValueCount{ static_cast<uint32_t>(clearValues.size()) },
				.pClearValues{ &clearValues[0U] },
			};

			VK_CHECK(vkBeginCommandBuffer(
				commandBuffer, 
				&commandBufferBeginInfo));

			vkCmdBeginRenderPass(
				commandBuffer, 
				&renderPassBeginInfo, 
				VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdEndRenderPass(
				commandBuffer);

			VK_CHECK(vkEndCommandBuffer(
				commandBuffer));
		}

		GraphicsContext::PerformSingleSubmitMemoryTransferOperation([](VkCommandBuffer lolz) {
			CIN_UNUSED(lolz);
			});

		VK_CHECK(vkQueueSubmit(
			GraphicsContext::GetGraphicsQueue(),
			1U,
			&submitInfo,
			imageInFlightFence));

		const VkPresentInfoKHR presentInfo{
			.sType{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR },
			.pNext{ nullptr },
			.waitSemaphoreCount{ 1U },
			.pWaitSemaphores{ &renderCompleteSemaphore }, /* Don't present before the image is rendered to */
			.swapchainCount{ 1U },
			.pSwapchains{ &m_Handle },
			.pImageIndices{ &m_ImageIndex },
			.pResults{ nullptr },
		};

		/* TODO: Add support for present queue */
		const VkResult result{
			vkQueuePresentKHR(
				GraphicsContext::GetPresentQueue(),
				&presentInfo) };

		switch (result)
		{
			case VK_SUCCESS:
			{
			} break;

			case VK_ERROR_OUT_OF_DATE_KHR:
			{
				GraphicsContext::ResizeSwapchain();
			} break;

			case VK_SUBOPTIMAL_KHR:
			{
				GraphicsContext::ResizeSwapchain();
			} break;

			case VK_ERROR_SURFACE_LOST_KHR:
			{
				GraphicsContext::RecreateSurface();
				return;
			} break;

			default:
			{
				GraphicsContext::RecreateSurface();
				CIN_WARN("Unhandled present result: {}", VKResultToString(result));
			} break;
		}

		m_FrameIndex = (m_FrameIndex + 1U) % m_FramesInFlight;
	}

	void Swapchain::Cleanup()
	{
		for (uint32_t i{ 0U }; i < m_ImageViews.size(); ++i)
			vkDestroyImageView(
				GraphicsContext::GetDevice(),
				m_ImageViews[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0U }; i < m_Framebuffers.size(); ++i)
			vkDestroyFramebuffer(
				GraphicsContext::GetDevice(),
				m_Framebuffers[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0U }; i < m_Semaphores.ImageAvailable.size(); ++i)
			vkDestroySemaphore(
				GraphicsContext::GetDevice(),
				m_Semaphores.ImageAvailable[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0U }; i < m_Semaphores.RenderingFinished.size(); ++i)
			vkDestroySemaphore(
				GraphicsContext::GetDevice(),
				m_Semaphores.RenderingFinished[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0U }; i < m_Fences.InFlightFences.size(); ++i)
			vkDestroyFence(
				GraphicsContext::GetDevice(),
				m_Fences.InFlightFences[i],
				GraphicsContext::GetAllocator());

		vkDestroyRenderPass(
			GraphicsContext::GetDevice(),
			m_RenderPass,
			GraphicsContext::GetAllocator());
	}
}