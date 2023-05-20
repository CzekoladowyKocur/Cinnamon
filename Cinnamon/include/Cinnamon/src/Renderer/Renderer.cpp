#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Surface.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/Material.hpp"
#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/UniformBuffer.hpp"
#include "Cinnamon/include/Renderer/DescriptorPool.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"
#include "Cinnamon/include/Core/Window.hpp"

namespace Cinnamon {
	Renderer::Renderer(const STL::Unique<Window>& windowContext) noexcept
		:
		m_Device(STL::MakeUnique<Device>(windowContext)),
		m_Allocator(STL::MakeUnique<VulkanAllocator>(m_Device)),
		m_Swapchain(STL::MakeUnique<Swapchain>(m_Device, windowContext)),
		m_DescriptorPool(STL::MakeUnique<DescriptorPool>(m_Device, m_Swapchain->GetImageCount()))
	{}

	Renderer::~Renderer() noexcept
	{
		m_DescriptorPool.reset();
		m_Swapchain.reset();
		m_Allocator.reset();
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
		CIN_ASSERT(framebuffer);
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		
		uint32_t framebufferWidth, framebufferHeight;
		VkFramebuffer framebufferHandle;
		VkRenderPass renderPass;
		const auto [width, height]{ framebuffer->GetSize() };
		framebufferWidth = width;
		framebufferHeight = height;

		framebufferHandle = framebuffer->GetHandle();
		renderPass = framebuffer->GetRenderPass();

		STL::Vector<VkClearValue> clearValues;
		for (size_t i{ 0U }; i < framebuffer->GetColorAttachmentCount(); ++i)
			clearValues.push_back(VkClearValue{ { framebuffer->GetColorAttachmentClearValue(i) } });
		
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
			.y{ static_cast<float>(framebufferHeight) },
			.width{ static_cast<float>(framebufferWidth) },
			.height{ -static_cast<float>(framebufferHeight) },
			.minDepth{ 0.0f },
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

	void Renderer::Clear(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
		const STL::Unique<Framebuffer>& framebuffer)
	{
		/* If framebuffer is cleared on load, simply skip the call to clean attachments. */
		if (not framebuffer->IsClearedOnLoad())
		{
			const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
			const VkCommandBuffer commandBuffer{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
			const auto [framebufferWidth, framebufferHeight] { framebuffer->GetSize() };

			STL::Vector<VkClearAttachment> clearAttachments;
			STL::Vector<VkClearRect> clearRectangles;
			for (size_t i{ 0U }; i < framebuffer->GetColorAttachmentCount(); ++i)
			{
				clearAttachments.emplace_back
				(
					VkClearAttachment
					{
						.aspectMask{ VK_IMAGE_ASPECT_COLOR_BIT },
						.colorAttachment{ static_cast<uint32_t>(i) },
						.clearValue{ { framebuffer->GetColorAttachmentClearValue(i) }}
					}
				);

				clearRectangles.emplace_back
				(
					VkClearRect
					{
						.rect
						{
							.offset{ 0, 0 },
							.extent{ framebufferWidth, framebufferHeight }
						},
						.baseArrayLayer{ 0U },
						.layerCount{ 1U },
					}
				);
			}

			vkCmdClearAttachments(
				commandBuffer,
				static_cast<uint32_t>(clearAttachments.size()),
				clearAttachments.data(),
				static_cast<uint32_t>(clearRectangles.size()),
				clearRectangles.data());
		}
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
		const STL::Unique<Material>& material,
		const uint32_t indexCount)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		const VkPipeline graphicsPipelineHandle{ pipeline->GetHandle() };
		const VkCommandBuffer commandBufferHandle{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		const VkBuffer vertexBufferHandle{ vertexBuffer->GetHandle() };
		const VkBuffer indexBufferHandle{ indexBuffer->GetHandle() };
		const STL::Unique<Shader>& shader{ pipeline->GetShader() };

		constexpr VkDeviceSize offsets[1U]{ 0U };
		/* Bind vertex buffer. */
		vkCmdBindVertexBuffers(
			commandBufferHandle,
			0, 1,
			&vertexBufferHandle,
			offsets);

		/* Bind index buffer. */
		vkCmdBindIndexBuffer(
			commandBufferHandle,
			indexBufferHandle,
			offsets[0U],
			VK_INDEX_TYPE_UINT32);

		/* Bind the graphics pipeline. */
		vkCmdBindPipeline(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineHandle);

		/* Allocate the descriptor sets needed for this shader. */
		shader->AllocateDescriptorSets(m_DescriptorPool->GetPool(frameIndex));
		material->Invalidate();

		STL::Vector<VkDescriptorSet> descriptorSets;
		for (const auto& [descriptorSetIndex, handle] : pipeline->GetShader()->GetDescriptorSetHandles())
			descriptorSets.push_back(handle);

		vkCmdBindDescriptorSets(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->GetLayout(),
			0,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr);

		vkCmdDrawIndexed(
			commandBufferHandle,
			static_cast<uint32_t>(indexCount),
			1U,
			0U,
			0U,
			0U);
	}

	void Renderer::RenderGeometry(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
		const STL::Unique<UniformBuffer>& UBO,
		const STL::Unique<VertexBuffer>& vertexBuffer,
		const STL::Unique<IndexBuffer>& indexBuffer,
		const STL::Unique<Pipeline>& pipeline,
		const STL::Unique<Material>& material,
		const uint32_t indexCount)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		const VkPipeline graphicsPipelineHandle{ pipeline->GetHandle() };
		const VkCommandBuffer commandBufferHandle{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		const VkBuffer vertexBufferHandle{ vertexBuffer->GetHandle() };
		const VkBuffer indexBufferHandle{ indexBuffer->GetHandle() };
		const STL::Unique<Shader>& shader{ pipeline->GetShader() };

		constexpr VkDeviceSize offsets[1U]{ 0U };
		/* Bind vertex buffer. */
		vkCmdBindVertexBuffers(
			commandBufferHandle,
			0, 1,
			&vertexBufferHandle,
			offsets);

		/* Bind index buffer. */
		vkCmdBindIndexBuffer(
			commandBufferHandle,
			indexBufferHandle,
			offsets[0U],
			VK_INDEX_TYPE_UINT32);

		/* Bind the graphics pipeline. */
		vkCmdBindPipeline(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineHandle);

		/* Allocate the descriptor sets needed for this shader. */
		shader->AllocateDescriptorSets(m_DescriptorPool->GetPool(frameIndex));
		/* Update the UBO */
		UpdateUBO(UBO, shader);
		
		material->Invalidate();

		STL::Vector<VkDescriptorSet> descriptorSets;
		for (const auto& [descriptorSetIndex, handle] : pipeline->GetShader()->GetDescriptorSetHandles())
			descriptorSets.push_back(handle);

		vkCmdBindDescriptorSets(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->GetLayout(),
			0,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr);

		vkCmdDrawIndexed(
			commandBufferHandle,
			static_cast<uint32_t>(indexCount),
			1U,
			0U,
			0U,
			0U);
	}

	void Renderer::RenderFullscreenQuad(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer,
		/*const STL::Unique<UniformBuffer>& UBO,*/
		const STL::Unique<Pipeline>& pipeline,
		const STL::Unique<Material>& material)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		const VkPipeline graphicsPipelineHandle{ pipeline->GetHandle() };
		const VkCommandBuffer commandBufferHandle{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		const STL::Unique<Shader>& shader{ pipeline->GetShader() };

		/* Bind the graphics pipeline. */
		vkCmdBindPipeline(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineHandle);

		/* Allocate the descriptor sets needed for this shader. */
		shader->AllocateDescriptorSets(m_DescriptorPool->GetPool(frameIndex));
		/* Update the material data */
		material->Invalidate();

		STL::Vector<VkDescriptorSet> descriptorSets;
		for (const auto& [descriptorSetIndex, handle] : pipeline->GetShader()->GetDescriptorSetHandles())
			descriptorSets.push_back(handle);
		
		vkCmdBindDescriptorSets(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->GetLayout(),
			0,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr);

		vkCmdDraw(
			commandBufferHandle,
			3U,
			1U,
			0U,
			0U);
	}

	void Renderer::RenderFullscreenQuad(
		const STL::Unique<RenderCommandBuffer>& renderCommandBuffer, 
		const STL::Unique<UniformBuffer>&		UBO, 
		const STL::Unique<Pipeline>&			pipeline, 
		const STL::Unique<Material>&			material)
	{
		const uint32_t frameIndex{ m_Swapchain->GetFrameIndex() };
		const VkPipeline graphicsPipelineHandle{ pipeline->GetHandle() };
		const VkCommandBuffer commandBufferHandle{ renderCommandBuffer->GetCommandBuffer(frameIndex) };
		const STL::Unique<Shader>& shader{ pipeline->GetShader() };

		/* Bind the graphics pipeline. */
		vkCmdBindPipeline(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineHandle);

		/* Allocate the descriptor sets needed for this shader. */
		shader->AllocateDescriptorSets(m_DescriptorPool->GetPool(frameIndex));
		/* Update the material data */
		material->Invalidate();
		/* Update the UBO */
		UpdateUBO(UBO, shader);

		STL::Vector<VkDescriptorSet> descriptorSets;
		for (const auto& [descriptorSetIndex, handle] : pipeline->GetShader()->GetDescriptorSetHandles())
			descriptorSets.push_back(handle);

		vkCmdBindDescriptorSets(
			commandBufferHandle,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->GetLayout(),
			0,
			static_cast<uint32_t>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr);

		vkCmdDraw(
			commandBufferHandle,
			6U,
			1U,
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

	const STL::Unique<VulkanAllocator>& Renderer::GetAllocator() const
	{
		CIN_ASSERT(m_Allocator);
		return m_Allocator;
	}

	void Renderer::UpdateUBO(
		const STL::Unique<UniformBuffer>& UBO,
		const STL::Unique<Shader>& shader)
	{
		/* Shader descriptor set 0 is reserved for UBOs */
		if (shader->HasDescriptorSet(0U))
		{
			const auto& uboDescriptorSet{ shader->GetDescriptorSets().at(0U) };

			STL::Vector<VkWriteDescriptorSet> writeDescriptorSets;
			for (const auto& [uniformBufferBinding, uniformBuffer] : uboDescriptorSet.UniformBuffers)
			{
				const VkDescriptorSet uboDescriptorSetHandle{ shader->GetDescriptorSetHandle(0U) };
				writeDescriptorSets.emplace_back
				(
					VkWriteDescriptorSet
					{
						.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
						.pNext{ nullptr },
						.dstSet{ uboDescriptorSetHandle },
						.dstBinding{ uniformBufferBinding },
						.dstArrayElement{ 0U },
						.descriptorCount{ 1U },
						.descriptorType{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
						.pImageInfo{ nullptr },
						.pBufferInfo{ &UBO->GetDescriptorBufferInfo()},
						.pTexelBufferView{ nullptr }
					}
				);
			}

			vkUpdateDescriptorSets(
				m_Device->GetLogicalDevice(),
				static_cast<uint32_t>(writeDescriptorSets.size()),
				writeDescriptorSets.data(),
				0,
				nullptr);
		}
		else
			CIN_WARN("Failed to update uniform buffer: the shader has no descriptor set 0");
	}
}