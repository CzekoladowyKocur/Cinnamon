#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/DescriptorPool.hpp"
#include "Cinnamon/include/Core/Window.hpp"

/* Temporary */
#include "Cinnamon/include/GUI/GUI.hpp"
#include "ThirdParty/imgui/imgui.h"

namespace Cinnamon {
	Renderer::Renderer(const STL::Unique<Window>& windowContext) noexcept
		:
		m_Device(STL::MakeUnique<Device>(windowContext)),
		m_Swapchain(STL::MakeUnique<Swapchain>(m_Device, windowContext)),
		m_DescriptorPool(STL::MakeUnique<DescriptorPool>(m_Device, m_Swapchain->GetImageCount()))
	{}

	Renderer::~Renderer() noexcept
	{
		m_DescriptorPool.reset();
		m_Swapchain.reset();
		m_Device.reset();
	}

	void Renderer::BeginFrame()
	{
		m_Swapchain->AcquireNextSwapchainImage();
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		m_DescriptorPool->ResetPool(frameIndex);
	}

	void Renderer::EndFrame()
	{
		m_Swapchain->PresentSwapchainImage();
	}

	void Renderer::BeginRenderPass(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
		const STL::Unique<Framebuffer>& framebuffer)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		
		uint32_t framebufferWidth, framebufferHeight;
		VkFramebuffer framebufferHandle;
		VkRenderPass renderPass;
		if (framebuffer)
		{
			const auto [width, height]{ framebuffer->GetSize() };
			framebufferWidth = width;
			framebufferHeight = height;

			framebufferHandle = framebuffer->GetHandle();
			renderPass = framebuffer->GetRenderPass();
		}
		else
		{
			const auto [width, height] { m_Swapchain->GetExtent() };
			framebufferWidth = width;
			framebufferHeight = height;
			
			renderPass = m_Swapchain->GetRenderPass();
			framebufferHandle = m_Swapchain->GetCurrentFramebuffer();
		}

		constexpr std::array<VkClearValue, 1> clearValues{ { { 0.15f, 0.95f, 0.15f, 1.0f } } };
		
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.framebuffer = framebufferHandle;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.extent = { framebufferWidth, framebufferHeight };
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.pNext = nullptr;

		VkCommandBuffer commandBuffer = renderCommandBuffer->GetCommandBuffer(frameIndex);
		
		vkCmdBeginRenderPass(
			commandBuffer,
			&renderPassBeginInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		/* Framebuffer viewport */
		VkViewport viewport;
		viewport.width = static_cast<float>(framebufferWidth);
		viewport.height = -(static_cast<float>(framebufferHeight));
		viewport.x = 0.0f;
		viewport.y = static_cast<float>(framebufferHeight);
		viewport.maxDepth = 1.0f;
		viewport.minDepth = 0.0f;

		VkRect2D scissor;
		scissor.extent = { framebufferWidth, framebufferHeight };
		scissor.offset = { 0, 0 };

		std::vector<VkClearAttachment> attachmentClears;
		std::vector<VkClearRect> clearRectangles;

		VkClearAttachment& clear = attachmentClears.emplace_back(VkClearAttachment{});
		clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		clear.colorAttachment = 0;
		clear.clearValue = clearValues[0];

		auto& rect = clearRectangles.emplace_back(VkClearRect{});
		rect.rect.extent = { framebufferWidth, framebufferHeight };
		rect.rect.offset = { 0, 0 };
		rect.layerCount = 1;
		rect.baseArrayLayer = 0;

		vkCmdClearAttachments(
			commandBuffer,
			static_cast<uint32_t>(attachmentClears.size()),
			attachmentClears.data(),
			static_cast<uint32_t>(attachmentClears.size()),
			clearRectangles.data());

		/* Dynamic state */
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::EndRenderPass(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };

		VkCommandBuffer vkCommandBuffer = renderCommandBuffer->GetCommandBuffer(frameIndex);
		vkCmdEndRenderPass(vkCommandBuffer);
	}

	void Renderer::RenderGeometry(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
		const STL::Unique<VertexBuffer>& vertexBuffer,
		const STL::Unique<IndexBuffer>& indexBuffer,
		const STL::Unique<Pipeline>& pipeline,
		const uint32_t indexCount)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		/* Native vulkan */
		VkPipeline graphicsPipeline = pipeline->GetHandle();
		VkCommandBuffer commandBuffer = renderCommandBuffer->GetCommandBuffer(frameIndex);
		
		VkBuffer vertexBufferHandle = vertexBuffer->GetHandle();
		VkBuffer indexBufferHandle = indexBuffer->GetHandle();

		VkDeviceSize offsets[1U]{ 0U };

		vkCmdBindVertexBuffers(
			commandBuffer,
			0, 1,
			&vertexBufferHandle,
			offsets);

		vkCmdBindIndexBuffer(
			commandBuffer,
			indexBufferHandle,
			offsets[0U],
			VK_INDEX_TYPE_UINT32);

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipeline);

		vkCmdDrawIndexed(
			commandBuffer,
			static_cast<uint32_t>(indexCount),
			1U, 
			0U, 
			0U, 
			0U);
	}

	void Renderer::SetClearColor(
		const float r, 
		const float g, 
		const float b, 
		const float a)
	{
		m_Swapchain->SetClearColor(r, g, b, a);
	}

	void Renderer::SetViewportSize(
		const uint32_t width, 
		const uint32_t height)
	{
		m_Swapchain->Recreate(width, height);
	}

	uint32_t Renderer::GetFrameIndex() const
	{
		return m_Swapchain->GetFrameIndex();
	}

	const STL::Unique<Device>& Renderer::GetDevice() const
	{
		CIN_ASSERT(m_Device);
		return m_Device;
	}

	const STL::Unique<Swapchain>& Renderer::GetSwapchain() const
	{
		CIN_ASSERT(m_Swapchain);
		return m_Swapchain;
	}

	const STL::Unique<DescriptorPool>& Renderer::GetDescriptorPool() const
	{
		CIN_ASSERT(m_DescriptorPool);
		return m_DescriptorPool;
	}
}