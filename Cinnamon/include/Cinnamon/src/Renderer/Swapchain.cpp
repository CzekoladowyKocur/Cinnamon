#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"

namespace Cinnamon {
	Swapchain::Swapchain(
		const STL::Unique<Device>& device,
		const STL::Unique<Window>& window) noexcept
		:
		m_Device(device),
		m_Surface(Platform::RetrieveWindowSurface(window)),
		m_Handle(VK_NULL_HANDLE),
		m_CachedSwapchain(VK_NULL_HANDLE),
		m_RenderPass(VK_NULL_HANDLE),
		m_CommandPool(VK_NULL_HANDLE),
		m_SurfaceFormat({ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }),
		m_PresentMode(VK_PRESENT_MODE_FIFO_KHR), /* The only one guaranteed by specification */
		m_SurfaceCapabilities({}),
		m_Extent({ 0U, 0U }),
		m_ClearColor({ .color { .float32 { 0.3f, 0.1f, 0.12f, 1.0f } }, }),
		m_MinimalImageCount(0U),
		m_SurfaceUpdated(false),
		m_Images({}),
		m_ImageViews({}),
		m_Framebuffers({}),
		m_Semaphores({}),
		m_Fences({}),
		m_ImageIndex(0U),
		m_FrameIndex(0U),
		m_FramesInFlight(2U),
		m_RecordFunction(nullptr)
	{
		const QueueFamilies& queueFamilies{ m_Device->GetQueueFamilies() };
		const VkCommandPoolCreateInfo commandPoolCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT },
			.queueFamilyIndex{ queueFamilies.Graphics },
		};

		VK_CHECK(vkCreateCommandPool(
			m_Device->GetLogicalDevice(),
			&commandPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_CommandPool));

		const auto [width, height] { window->GetSize() };
		Create(width, height);
	}

	Swapchain::~Swapchain() noexcept
	{
		VK_CHECK(vkDeviceWaitIdle(
			m_Device->GetLogicalDevice()));

		Cleanup();
		vkDestroyCommandPool(
			m_Device->GetLogicalDevice(),
			m_CommandPool,
			GraphicsContext::GetAllocator());

		vkDestroySwapchainKHR(
			m_Device->GetLogicalDevice(),
			m_Handle,
			GraphicsContext::GetAllocator());

		Platform::DestroySurface(m_Surface);
	}

	void Swapchain::Create(const uint32_t width, const uint32_t height)
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
				m_Device->GetPhysicalDevice(),
				m_Surface,
				&availableSurfaceFormatCount,
				nullptr));

			CIN_ASSERT(availableSurfaceFormatCount > 0U, "No available surface format!");
			STL::Vector<VkSurfaceFormatKHR> availableSurfaceFormats(availableSurfaceFormatCount);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
				m_Device->GetPhysicalDevice(),
				m_Surface,
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
				m_Device->GetPhysicalDevice(),
				m_Surface,
				&availablePresentModeCount,
				nullptr));

			CIN_ASSERT(availablePresentModeCount > 0U, "No available present mode");
			STL::Vector<VkPresentModeKHR> availablePresentModes(availablePresentModeCount);

			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
				m_Device->GetPhysicalDevice(),
				m_Surface,
				&availablePresentModeCount,
				&availablePresentModes[0]));

			bool found{ false };
			const VkPresentModeKHR desiredPresentMode{ Platform::GetDesiredSurfacePresentMode(m_Surface) };
			for (const VkPresentModeKHR availablePresentMode : availablePresentModes)
				if (availablePresentMode == desiredPresentMode)
				{
					m_PresentMode = desiredPresentMode;
					found = true;
					break;
				}

			if (!found)
				m_PresentMode = VK_PRESENT_MODE_FIFO_KHR; /* Guaranteed by specification */
		}

		/* Get surface capabilities */
		{
			VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				m_Device->GetPhysicalDevice(),
				m_Surface,
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

		m_MinimalImageCount = m_SurfaceCapabilities.minImageCount;
		uint32_t imageCount{ m_SurfaceCapabilities.minImageCount + 1U };
		if (m_SurfaceCapabilities.maxImageCount > 0U && imageCount > m_SurfaceCapabilities.maxImageCount)
			imageCount = m_SurfaceCapabilities.maxImageCount;

		const bool queuesFamiliesShared{ m_Device->PresentAndGraphicsFamiliesShared() };
		const QueueFamilies& queueFamilies{ m_Device->GetQueueFamilies() };
		const STL::Array<uint32_t, 2> queueFamilyIndices{ queueFamilies.Graphics, queueFamilies.Present };

		VkCompositeAlphaFlagBitsKHR compositeAlpha{ VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR };
		if (m_SurfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		else if (m_SurfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
			compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

		const VkSwapchainCreateInfoKHR swapchainCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR },
			.pNext{ nullptr },
			.flags{ 0U },
			.surface{ m_Surface },
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
			.compositeAlpha{ compositeAlpha },
			.presentMode{ m_PresentMode },
			.clipped{ VK_TRUE },
			.oldSwapchain{ m_CachedSwapchain },
		};

		VK_CHECK(vkCreateSwapchainKHR(
			m_Device->GetLogicalDevice(),
			&swapchainCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));

		/* Main color attachment */
		const VkAttachmentDescription colorAttachmentDescription
		{
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

		constexpr VkAttachmentReference colorAttachmentReference
		{
			.attachment{ 0U },
			.layout{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } /* Specifies the layout of the attachment during subpass */
		};

		const VkSubpassDescription subpassDescription
		{
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

		constexpr VkSubpassDependency subpassDependency
		{
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

		const VkRenderPassCreateInfo renderPassCreateInfo
		{
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
			m_Device->GetLogicalDevice(),
			&renderPassCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_RenderPass));

		/* Retrieve vulkan-side created swapchain images */
		VK_CHECK(vkGetSwapchainImagesKHR(
			m_Device->GetLogicalDevice(),
			m_Handle,
			&imageCount,
			nullptr));

		CIN_ASSERT(imageCount > 0U, "Invalid image count");
		m_Images.resize(imageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(
			m_Device->GetLogicalDevice(),
			m_Handle,
			&imageCount,
			&m_Images[0U]));

		VkImageViewCreateInfo imageViewCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ 0U },
			.image{ nullptr }, /* Set in loop */
			.viewType{ VK_IMAGE_VIEW_TYPE_2D },
			.format{ m_SurfaceFormat.format },
			.components
			{
				.r { VK_COMPONENT_SWIZZLE_R },
				.g { VK_COMPONENT_SWIZZLE_G },
				.b { VK_COMPONENT_SWIZZLE_B },
				.a { VK_COMPONENT_SWIZZLE_A },
			},
			.subresourceRange
			{
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
				m_Device->GetLogicalDevice(),
				&imageViewCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_ImageViews[i]));
		}

		VkFramebufferCreateInfo framebufferCreateInfo
		{
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
				m_Device->GetLogicalDevice(),
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
			m_Device->GetLogicalDevice(),
			&commandBufferAllocateInfo,
			m_CommandBuffers.data()));

		m_Fences.InFlightFences.resize(m_FramesInFlight);
		m_Semaphores.ImageAvailable.resize(m_FramesInFlight);
		m_Semaphores.RenderingFinished.resize(m_FramesInFlight);

		constexpr VkFenceCreateInfo fenceCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_FENCE_CREATE_SIGNALED_BIT },
		};

		constexpr VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO },
			.pNext{ nullptr },
			.flags{ VK_SEMAPHORE_TYPE_BINARY },
		};

		for (uint32_t i{ 0U }; i < m_FramesInFlight; ++i)
		{
			VK_CHECK(vkCreateFence(
				m_Device->GetLogicalDevice(),
				&fenceCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Fences.InFlightFences[i]));

			VK_CHECK(vkCreateSemaphore(
				m_Device->GetLogicalDevice(),
				&semaphoreCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Semaphores.ImageAvailable[i]));

			VK_CHECK(vkCreateSemaphore(
				m_Device->GetLogicalDevice(),
				&semaphoreCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Semaphores.RenderingFinished[i]));
		}

		CIN_TRACE("Created {0}-buffered swapchain with present and graphics queue families {1}", imageCount, queuesFamiliesShared ? "shared" : "not shared");
	}

	void Swapchain::Recreate(const uint32_t width, const uint32_t height)
	{
		VK_CHECK(vkDeviceWaitIdle(
			m_Device->GetLogicalDevice()));

		/* If surface hasn't changed, cache the handle and use it in creation */
		m_CachedSwapchain = !m_SurfaceUpdated ? m_Handle : VK_NULL_HANDLE;
		/* Store the old handle to delete it (always gets deleted) */
		const VkSwapchainKHR oldHandle{ m_Handle };
		/* Unsignal surface update */
		m_SurfaceUpdated = false;

		Cleanup();
		Create(width, height);
		vkDestroySwapchainKHR(
			m_Device->GetLogicalDevice(),
			oldHandle,
			GraphicsContext::GetAllocator());
	}

	void Swapchain::Cleanup()
	{
		for (size_t i{ 0U }; i < m_Images.size(); ++i)
		{
			CIN_ASSERT(m_ImageViews.size() == m_Images.size());
			CIN_ASSERT(m_Framebuffers.size() == m_Images.size());

			vkDestroyImageView(
				m_Device->GetLogicalDevice(),
				m_ImageViews[i],
				GraphicsContext::GetAllocator());

			vkDestroyFramebuffer(
				m_Device->GetLogicalDevice(),
				m_Framebuffers[i],
				GraphicsContext::GetAllocator());
		}

		for (uint32_t i{ 0U }; i < m_FramesInFlight; ++i)
		{
			CIN_ASSERT(static_cast<uint32_t>(m_Semaphores.ImageAvailable.size()) == m_FramesInFlight);
			CIN_ASSERT(static_cast<uint32_t>(m_Semaphores.RenderingFinished.size()) == m_FramesInFlight);
			CIN_ASSERT(static_cast<uint32_t>(m_Fences.InFlightFences.size()) == m_FramesInFlight);

			vkDestroySemaphore(
				m_Device->GetLogicalDevice(),
				m_Semaphores.ImageAvailable[i],
				GraphicsContext::GetAllocator());

			vkDestroySemaphore(
				m_Device->GetLogicalDevice(),
				m_Semaphores.RenderingFinished[i],
				GraphicsContext::GetAllocator());
			
			vkDestroyFence(
				m_Device->GetLogicalDevice(),
				m_Fences.InFlightFences[i],
				GraphicsContext::GetAllocator());
		}

		vkDestroyRenderPass(
			m_Device->GetLogicalDevice(),
			m_RenderPass,
			GraphicsContext::GetAllocator());
	}

	void Swapchain::SetClearColor(
		const float r, 
		const float g, 
		const float b,
		const float a)
	{
		m_ClearColor = VkClearValue{ .color {.float32 { r, g, b, a } }, };
	}

	void Swapchain::AcquireNextSwapchainImage()
	{
		const VkFence imageInFlightFence{ m_Fences.InFlightFences[m_FrameIndex] };
		const VkSemaphore imageAvailableSemaphore{ m_Semaphores.ImageAvailable[m_FrameIndex] };

		VkResult result;
		do 
		{
			result = vkGetFenceStatus(
				m_Device->GetLogicalDevice(),
				imageInFlightFence);
		} while (result == VK_NOT_READY);

		do {
			result = vkAcquireNextImageKHR(
				m_Device->GetLogicalDevice(),
				m_Handle,
				std::numeric_limits<std::uint64_t>::max(),
				imageAvailableSemaphore,
				VK_NULL_HANDLE, /* Fence here instead? */
				&m_ImageIndex);

			switch (result)
			{
				case VK_SUCCESS:
				{
				} break;

				case VK_NOT_READY:
				{
				} break;

				case VK_ERROR_OUT_OF_DATE_KHR:
				{
					Recreate(m_Extent.width, m_Extent.height);
				} break;

				case VK_SUBOPTIMAL_KHR:
				{
					Recreate(m_Extent.width, m_Extent.height);
				} break;

				case VK_ERROR_SURFACE_LOST_KHR:
				{
					Recreate(m_Extent.width, m_Extent.height);
					return;
				} break;

				default:
				{
					m_Surface = Platform::RecreateWindowSurface(m_Surface);
					m_SurfaceUpdated = true;

					Recreate(m_Extent.width, m_Extent.height);
					CIN_WARN("Unhandled acquire result: {0}", VKResultToString(result));
				} break;
			}
		} while (result != VK_SUCCESS);
	}

	void Swapchain::PresentSwapchainImage()
	{
		/* Signals images in flight */
		const VkFence imageInFlightFence{ m_Fences.InFlightFences[m_FrameIndex] };
		/* Signals whether presenting the image has finished (image is available) */
		const VkSemaphore imageAvailableSemaphore{ m_Semaphores.ImageAvailable[m_FrameIndex] };
		/* Signals whether rendering the image has finished (image was rendered to) */
		const VkSemaphore renderCompleteSemaphore{ m_Semaphores.RenderingFinished[m_FrameIndex] };
		/* Current frmae command buffer */
		const VkCommandBuffer commandBuffer{ m_CommandBuffers[m_FrameIndex] };

		/* Unsignal the fence */
		VK_CHECK(vkResetFences(
			m_Device->GetLogicalDevice(),
			1U,
			&imageInFlightFence));

		/* Color attachment output */
		constexpr STL::Array<VkPipelineStageFlags, 1U> waitStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		const VkSubmitInfo submitInfo
		{
			.sType{ VK_STRUCTURE_TYPE_SUBMIT_INFO },
			.pNext{ nullptr},
			.waitSemaphoreCount{ 1U },
			.pWaitSemaphores{ &imageAvailableSemaphore },
			.pWaitDstStageMask{ &waitStages[0U] },
			.commandBufferCount{ 1U },
			.pCommandBuffers{ &commandBuffer },
			.signalSemaphoreCount{ 1U },
			.pSignalSemaphores{ &renderCompleteSemaphore },
		};

		CIN_ASSERT(m_RecordFunction);
		if (m_RecordFunction)
		{
			m_RecordFunction(
				commandBuffer, 
				m_Framebuffers[m_ImageIndex], 
				m_RenderPass, 
				m_Extent);
		}
#if 0
		else if(false)
		/* Record default clear command buffers */
		{
			const STL::Array<VkClearValue, 1U> clearValues{ m_ClearColor };
		
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
				.renderArea
				{
					.offset
					{
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
#endif
		VK_CHECK(vkQueueSubmit(
			m_Device->GetQueues().Graphics,
			1U,
			&submitInfo,
			imageInFlightFence));

		const VkPresentInfoKHR presentInfo
		{
			.sType{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR },
			.pNext{ nullptr },
			.waitSemaphoreCount{ 1U },
			.pWaitSemaphores{ &renderCompleteSemaphore }, /* Don't present before the image is rendered to */
			.swapchainCount{ 1U },
			.pSwapchains{ &m_Handle },
			.pImageIndices{ &m_ImageIndex },
			.pResults{ nullptr },
		};

		const VkResult result
		{
			vkQueuePresentKHR(
				m_Device->GetQueues().Present,
				&presentInfo) 
		};

		switch (result)
		{
			case VK_SUCCESS:
			{
			} break;

			case VK_ERROR_OUT_OF_DATE_KHR:
			{
				Recreate(m_Extent.width, m_Extent.height);
			} break;

			case VK_SUBOPTIMAL_KHR:
			{
				Recreate(m_Extent.width, m_Extent.height);
			} break;

			case VK_ERROR_SURFACE_LOST_KHR:
			{
				m_Surface = Platform::RecreateWindowSurface(m_Surface);
				m_SurfaceUpdated = true;
				
				Recreate(m_Extent.width, m_Extent.height);
				return;
			} break;

			default:
			{
				m_Surface = Platform::RecreateWindowSurface(m_Surface);
				m_SurfaceUpdated = true;

				Recreate(m_Extent.width, m_Extent.height);
				CIN_WARN("Unhandled present result: {}", VKResultToString(result));
			} break;
		}

		m_FrameIndex = (m_FrameIndex + 1U) % m_FramesInFlight;
	}

	void Swapchain::RecordCommands(const SwapchainCommandRecordCallback recordFunction)
	{
		m_RecordFunction = recordFunction;
	}

	uint32_t Swapchain::GetImageCount() const
	{
		CIN_ASSERT(m_Images.size() > 0, "Invalid image count");
		return static_cast<uint32_t>(m_Images.size());
	}

	uint32_t Swapchain::GetImageMinimalCount() const
	{
		return m_MinimalImageCount;
	}

	uint32_t Swapchain::GetImageIndex() const
	{
		return m_ImageIndex;
	}

	uint32_t Swapchain::GetFrameIndex() const
	{
		return m_FrameIndex;
	}

	VkExtent2D Swapchain::GetExtent() const
	{
		return m_Extent;
	}

	VkRenderPass Swapchain::GetRenderPass() const
	{
		return m_RenderPass;
	}

	VkFramebuffer Swapchain::GetCurrentFramebuffer() const
	{
		return m_Framebuffers[m_ImageIndex];
	}

	VkCommandBuffer Swapchain::GetCommandBuffer(const uint32_t frameIndex) const
	{
		return m_CommandBuffers[frameIndex];
	}

	VkFence Swapchain::GetWaitFence(const uint32_t frameIndex) const
	{
		return m_Fences.InFlightFences[frameIndex];
	}
}