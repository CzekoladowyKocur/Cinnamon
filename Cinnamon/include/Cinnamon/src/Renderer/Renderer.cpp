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
#include "Cinnamon/include/Renderer/Texture2D.hpp"
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
		const VkRenderPassBeginInfo renderPassBeginInfo
		{
			.sType{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO },
			.pNext{ nullptr },
			.renderPass{ renderPass },
			.framebuffer{ framebufferHandle },
			.renderArea
			{
				.offset
				{
					.x{ 0U },
					.y{ 0U }
				},
				.extent
				{
					.width{ framebufferWidth },
					.height{ framebufferHeight },
				}
			},
			.clearValueCount{ static_cast<uint32_t>(clearValues.size()) },
			.pClearValues{ clearValues.empty() ? nullptr : clearValues.data() },
		};

		const VkCommandBuffer commandBuffer{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		vkCmdBeginRenderPass(
			commandBuffer,
			&renderPassBeginInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		/* Framebuffer viewport */
		const VkViewport viewport
		{
			.x{ 0.0f },
			.y{ 0.0f },
			.width{ static_cast<float>(framebufferWidth) },
			.height{ static_cast<float>(framebufferHeight) },
			.minDepth{ 0U },
			.maxDepth{ 1.0f },
		};

		const VkRect2D scissor
		{
			.offset
			{
				.x{ 0 },
				.y{ 0 }
			},
			.extent
			{
				.width{ framebufferWidth },
				.height{ framebufferHeight },
			}
		};

		//std::vector<VkClearAttachment> attachmentClears;
		//std::vector<VkClearRect> clearRectangles;

		//VkClearAttachment& clear = attachmentClears.emplace_back(VkClearAttachment{});
		//clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//clear.colorAttachment = 0;
		//clear.clearValue = clearValues[0];
		//
		//auto& rect = clearRectangles.emplace_back(VkClearRect{});
		//rect.rect.extent = { framebufferWidth, framebufferHeight };
		//rect.rect.offset = { 0, 0 };
		//rect.layerCount = 1;
		//rect.baseArrayLayer = 0;

		//vkCmdClearAttachments(
		//	commandBuffer,
		//	static_cast<uint32_t>(attachmentClears.size()),
		//	attachmentClears.data(),
		//	static_cast<uint32_t>(attachmentClears.size()),
		//	clearRectangles.data());

		/* Dynamic state */
		vkCmdSetViewport(
			commandBuffer, 
			0, 
			1, 
			&viewport);

		vkCmdSetScissor(
			commandBuffer, 
			0, 
			1, 
			&scissor);
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
		const VkPipeline graphicsPipeline{ pipeline->GetHandle() };
		const VkCommandBuffer commandBuffer{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		const VkBuffer vertexBufferHandle{ vertexBuffer->GetHandle() };
		const VkBuffer indexBufferHandle{ indexBuffer->GetHandle() };
		constexpr VkDeviceSize offsets[1U]{ 0U };

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

	void Renderer::RenderGeometry(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer, 
		const STL::Unique<VertexBuffer>& vertexBuffer, 
		const STL::Unique<IndexBuffer>& indexBuffer, 
		const STL::Unique<Pipeline>& pipeline, 
		const STL::Unique<Shader>& shader,
		const Texture2D& texture, 
		const uint32_t indexCount)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		const VkPipeline graphicsPipeline{ pipeline->GetHandle() };
		const VkCommandBuffer commandBuffer{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		const VkBuffer vertexBufferHandle{ vertexBuffer->GetHandle() };
		const VkBuffer indexBufferHandle{ indexBuffer->GetHandle() };
		constexpr VkDeviceSize offsets[1U]{ 0U };

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

		const VkDescriptorImageInfo imageInfo 
		{ 
			.sampler{ texture.GetSampler() },
			.imageView{ texture.GetImageView() },
			.imageLayout{ texture.GetImageLayout() }
		};

		VkDescriptorSet dst{ shader->AllocateDescriptorSet(0U, m_DescriptorPool->GetPool(frameIndex)) };

		const VkWriteDescriptorSet descriptorWrite
		{
			.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
			.pNext{ nullptr },
			.dstSet{ dst },
			.dstBinding{ 0U },
			.dstArrayElement{ 0U },
			.descriptorCount{ 1U },
			.descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
			.pImageInfo{ &imageInfo },
			.pBufferInfo{ nullptr },
			.pTexelBufferView{ nullptr }
		};

		vkUpdateDescriptorSets(
			m_Device->GetLogicalDevice(), 
			1, 
			&descriptorWrite, 
			0, 
			nullptr);

		vkCmdBindDescriptorSets(
			commandBuffer, 
			VK_PIPELINE_BIND_POINT_GRAPHICS, 
			pipeline->GetLayout(),
			0, 
			1, 
			&dst, 
			0, 
			nullptr);

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