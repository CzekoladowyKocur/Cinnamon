#include "Cinnamon/include/Renderer/Swapchain.h"
#include "Cinnamon/include/Renderer/GraphicsContext.h"

namespace Cinnamon {
	Swapchain::Swapchain(const uint32_t width, const uint32_t height, VkSurfaceKHR surface) noexcept
		:
		m_SurfaceFormat({ VK_FORMAT_UNDEFINED, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }),
		m_PresentMode(VK_PRESENT_MODE_FIFO_KHR), /* The only one guaranteed by specification */
		m_SurfaceCapabilities({}),
		m_Extent({ width, height }),
		m_Handle(VK_NULL_HANDLE),
		m_ImageIndex(0U),
		m_FrameIndex(0U),
		m_FramesInFlight(2)
	{
		Create(width, height, surface);
	}

	Swapchain::~Swapchain() noexcept
	{
		VK_CHECK(vkDeviceWaitIdle(GraphicsContext::GetDevice()));

		vkDestroyCommandPool(
			GraphicsContext::GetDevice(),
			m_CommandPool,
			GraphicsContext::GetAllocator());

		Cleanup();
	}

	void Swapchain::Create(const uint32_t width, const uint32_t height, VkSurfaceKHR surface)
	{
		m_ImageIndex = 0;
		m_FrameIndex = 0;
		CIN_ASSERT(width >= 1 && width <= 15360, "Invalid swapchain width");
		CIN_ASSERT(height >= 1 && height <= 8640, "Invalid swapchain height");
		CIN_TRACE("Recreating swapchain: {0}, {1}", width, height);

		/* Pick surface format */
		{
			uint32_t availableSurfaceFormatCount{ 0 };
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availableSurfaceFormatCount,
				nullptr));

			CIN_ASSERT(availableSurfaceFormatCount > 0, "No available surface format!");
			STL::Vector<VkSurfaceFormatKHR> availableSurfaceFormats(availableSurfaceFormatCount);
			VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availableSurfaceFormatCount,
				&availableSurfaceFormats[0]));

			bool found{ false };
			for (const VkSurfaceFormatKHR availableFormat : availableSurfaceFormats)
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					found = true;
					m_SurfaceFormat = availableFormat;
					break;
				}

			if (!found)
				m_SurfaceFormat = availableSurfaceFormats[0];
		}

		/* Pick present mode */
		{
			uint32_t availablePresentModeCount{ 0 };
			VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
				GraphicsContext::GetPhysicalDevice(),
				surface,
				&availablePresentModeCount,
				nullptr));

			CIN_ASSERT(availablePresentModeCount > 0, "No available present mode");
			STL::Vector<VkPresentModeKHR> availablePresentModes(availablePresentModeCount);

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
			const auto& capabilities = m_SurfaceCapabilities;
			if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max() && capabilities.currentExtent.height != std::numeric_limits<std::uint32_t>::max())
				m_Extent = capabilities.currentExtent;
			else
			{
				VkExtent2D extent = {
					width,
					height
				};

				extent.width = CIN_CLAMP(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.width = CIN_CLAMP(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

				m_Extent = extent;
			}
		}

		uint32_t imageCount{ m_SurfaceCapabilities.minImageCount + 1 };
		if (m_SurfaceCapabilities.maxImageCount > 0 && imageCount > m_SurfaceCapabilities.maxImageCount)
			imageCount = m_SurfaceCapabilities.maxImageCount;

		VkSwapchainCreateInfoKHR swapchainCreateInfo;
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = surface;
		swapchainCreateInfo.imageFormat = m_SurfaceFormat.format;
		swapchainCreateInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.presentMode = m_PresentMode;
		swapchainCreateInfo.minImageCount = imageCount;
		swapchainCreateInfo.imageExtent = m_Extent;
		swapchainCreateInfo.imageArrayLayers = 1; /* One image layer */
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; /* Currently only graphics queue */
		swapchainCreateInfo.queueFamilyIndexCount = VK_QUEUE_FAMILY_IGNORED;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
		swapchainCreateInfo.preTransform = m_SurfaceCapabilities.currentTransform;
		swapchainCreateInfo.clipped = VK_TRUE;
		swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; /* TODO: Cache old handle */
		swapchainCreateInfo.flags = 0;
		swapchainCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreateSwapchainKHR(
			GraphicsContext::GetDevice(),
			&swapchainCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));

		VkAttachmentDescription colorAttachmentDescription;
		colorAttachmentDescription.format = m_SurfaceFormat.format;
		colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; /* is the layout the attachment image subresource will be in when a render pass instance begins. */
		colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; /* is the layout the attachment image subresource will be transitioned to when a render pass instance ends. */
		colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE; /* For presenting the image to surface */
		colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentDescription.flags = 0;

		VkAttachmentReference colorAttachmentReference;
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; /* is a VkImageLayout value specifying the layout the attachment uses during the subpass. */

		VkSubpassDescription subpassDescription;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.flags = 0;

		VkSubpassDependency subpassDependency;
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; /* is the subpass index of the first subpass in the dependency, or VK_SUBPASS_EXTERNAL. */
		subpassDependency.dstSubpass = 0; /* is the subpass index of the second subpass in the dependency, or VK_SUBPASS_EXTERNAL. */
		/*
		If srcSubpass is equal to VK_SUBPASS_EXTERNAL, the first synchronization scope includes commands that
		occur earlier in submission order than the vkCmdBeginRenderPass used to begin the render pass instance.
		Otherwise, the first set of commands includes all commands submitted as part of the subpass instance identified
		by srcSubpass and any load, store or multisample resolve operations on attachments used in
		srcSubpass. In either case, the first synchronization scope is limited to operations on the pipeline
		stages determined by the source stage mask specified by srcStageMask.
		*/
		subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask = 0;
		subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;
		renderPassCreateInfo.flags = 0;
		renderPassCreateInfo.pNext = nullptr;

		vkCreateRenderPass(
			GraphicsContext::GetDevice(),
			&renderPassCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_RenderPass);

		/* Retrieve vulkan-side created swapchain images */
		VK_CHECK(vkGetSwapchainImagesKHR(
			GraphicsContext::GetDevice(),
			m_Handle,
			&imageCount,
			nullptr));

		CIN_ASSERT(imageCount > 0, "Invalid image count");
		m_Images.resize(imageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(
			GraphicsContext::GetDevice(),
			m_Handle,
			&imageCount,
			&m_Images[0]));

		VkImageViewCreateInfo imageViewCreateInfo;
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = VK_NULL_HANDLE;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_SurfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.pNext = nullptr;

		m_ImageViews.resize(imageCount);
		for (uint32_t i{ 0 }; i < m_ImageViews.size(); ++i)
		{
			imageViewCreateInfo.image = m_Images[i];

			VK_CHECK(vkCreateImageView(
				GraphicsContext::GetDevice(),
				&imageViewCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_ImageViews[i]));
		}

		VkFramebufferCreateInfo framebufferCreateInfo;
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = VK_NULL_HANDLE;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.width = m_Extent.width;
		framebufferCreateInfo.height = m_Extent.height;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.flags = 0;
		framebufferCreateInfo.pNext = nullptr;

		m_Framebuffers.resize(imageCount);
		for (uint32_t i{ 0 }; i < m_Framebuffers.size(); ++i)
		{
			framebufferCreateInfo.pAttachments = &m_ImageViews[i];

			VK_CHECK(vkCreateFramebuffer(
				GraphicsContext::GetDevice(),
				&framebufferCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Framebuffers[i]));
		}

		VkCommandPoolCreateInfo commandPoolCreateInfo;
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.queueFamilyIndex = GraphicsContext::GetQueueFamily(GraphicsContext::EQueueFamily::Graphics);
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreateCommandPool(
			GraphicsContext::GetDevice(),
			&commandPoolCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_CommandPool));

		m_CommandBuffers.resize(imageCount);
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandPool = m_CommandPool;
		commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());
		commandBufferAllocateInfo.pNext = nullptr;

		VK_CHECK(vkAllocateCommandBuffers(
			GraphicsContext::GetDevice(),
			&commandBufferAllocateInfo,
			m_CommandBuffers.data()));

		m_Fences.InFlightFences.resize(m_FramesInFlight);
		m_Semaphores.ImageAvailable.resize(m_FramesInFlight);
		m_Semaphores.RenderingFinished.resize(m_FramesInFlight);

		VkFenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; /* Make it signaled (like a finished queue operation would) so no deadlock occurs */
		fenceCreateInfo.pNext = nullptr;

		VkSemaphoreCreateInfo semaphoreCreateInfo;
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.flags = VK_SEMAPHORE_TYPE_BINARY;
		semaphoreCreateInfo.pNext = nullptr;

		for (uint32_t i = 0; i < m_FramesInFlight; ++i)
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

		CIN_TRACE("Created swapchain with {0} images", imageCount);
	}

	void Swapchain::AcquireNextSwapchainImage()
	{
		VkFence imageInFlightFence{ m_Fences.InFlightFences[m_FrameIndex] };
		VkSemaphore presentCompleteSemaphore{ m_Semaphores.ImageAvailable[m_FrameIndex] };

		VK_CHECK(vkWaitForFences(
			GraphicsContext::GetDevice(),
			1,
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
			case VK_ERROR_OUT_OF_DATE_KHR:
			{
				//Resize();
			} break;

			case VK_SUBOPTIMAL_KHR:
			{
			} break;

			case VK_ERROR_SURFACE_LOST_KHR:
			{
				//GraphicsContext::RecreateSurface();
				//Resize();
				return;
			} break;
			}
		} while (result != VK_SUCCESS);

		/* Unsignal the fence */
		VK_CHECK(vkResetFences(
			GraphicsContext::GetDevice(),
			1,
			&imageInFlightFence));
	}

	void Swapchain::PresentSwapchainImage()
	{
		VkFence imageInFlightFence{ m_Fences.InFlightFences[m_FrameIndex] };
		VkSemaphore presentCompleteSemaphore{ m_Semaphores.ImageAvailable[m_FrameIndex] };
		VkSemaphore renderCompleteSemaphore{ m_Semaphores.RenderingFinished[m_FrameIndex] };
		VkCommandBuffer commandBuffer{ m_CommandBuffers[m_FrameIndex] };

		/* Color attachment output */
		constexpr STL::Array<VkPipelineStageFlags, 1> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphore; /* Will be signaled after image is acquired */
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphore; /* Will be signaled after executing the command buffer */
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.pNext = nullptr;

		{
			/* Record default clear command buffers */
			constexpr STL::Array<VkClearValue, 1> clearValues{
					{ {0.3f, 0.1f, 0.12f, 1.0f} }
			};

			VkCommandBufferBeginInfo commandBufferBeginInfo;
			commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			commandBufferBeginInfo.pInheritanceInfo = nullptr;
			commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			commandBufferBeginInfo.pNext = nullptr;

			VkRenderPassBeginInfo renderPassBeginInfo;
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = m_RenderPass;
			renderPassBeginInfo.framebuffer = m_Framebuffers[m_ImageIndex];
			renderPassBeginInfo.renderArea.extent = m_Extent;
			renderPassBeginInfo.renderArea.offset = { 0, 0 };
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();
			renderPassBeginInfo.pNext = nullptr;

			VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdEndRenderPass(commandBuffer);

			VK_CHECK(vkEndCommandBuffer(commandBuffer));
		}

		VK_CHECK(vkQueueSubmit(
			GraphicsContext::GetGraphicsQueue(),
			1,
			&submitInfo,
			imageInFlightFence));

		VkPresentInfoKHR presentInfo;
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_Handle;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderCompleteSemaphore; /* Wait till the image is rendered to */
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr;
		presentInfo.pNext = nullptr;

		/* TODO: Add support for present queue */
		VkResult result{
			vkQueuePresentKHR(
				GraphicsContext::GetGraphicsQueue(),
				&presentInfo) };

		switch (result)
		{
		case VK_SUCCESS:
		{
		} break;

		case VK_ERROR_OUT_OF_DATE_KHR:
		{
			//Resize();
		} break;

		case VK_SUBOPTIMAL_KHR:
		{
			//Resize();

		} break;

		case VK_ERROR_SURFACE_LOST_KHR:
		{
			//GraphicsContext::RecreateSurface();
			//Resize();
		} break;

		default:
		{
			CIN_WARN("Unhandled present result: {0}", VKResultToString(result));
		} break;
		}

		m_FrameIndex = (++m_FrameIndex) % m_FramesInFlight;
	}

	void Swapchain::Cleanup()
	{
		vkDestroySwapchainKHR(
			GraphicsContext::GetDevice(),
			m_Handle,
			GraphicsContext::GetAllocator());

		for (uint32_t i{ 0 }; i < m_ImageViews.size(); ++i)
			vkDestroyImageView(
				GraphicsContext::GetDevice(),
				m_ImageViews[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0 }; i < m_Framebuffers.size(); ++i)
			vkDestroyFramebuffer(
				GraphicsContext::GetDevice(),
				m_Framebuffers[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0 }; i < m_Semaphores.ImageAvailable.size(); ++i)
			vkDestroySemaphore(
				GraphicsContext::GetDevice(),
				m_Semaphores.ImageAvailable[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0 }; i < m_Semaphores.RenderingFinished.size(); ++i)
			vkDestroySemaphore(
				GraphicsContext::GetDevice(),
				m_Semaphores.RenderingFinished[i],
				GraphicsContext::GetAllocator());

		for (uint32_t i{ 0 }; i < m_Fences.InFlightFences.size(); ++i)
			vkDestroyFence(
				GraphicsContext::GetDevice(),
				m_Fences.InFlightFences[i],
				GraphicsContext::GetAllocator());

		vkDestroyRenderPass(
			GraphicsContext::GetDevice(),
			m_RenderPass,
			GraphicsContext::GetAllocator());

		//vkFreeCommandBuffers(
		//	GraphicsContext::GetDevice(),
		//	m_CommandPool,
		//	static_cast<uint32_t>(m_CommandBuffers.size()),
		//	&m_CommandBuffers[0]);
	}
}