#include "Cinnamon/include/Renderer/Renderer2D.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/Material.hpp"
#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/UniformBuffer.hpp"
#include "Cinnamon/include/Renderer/Geometry.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

namespace Cinnamon {
	constexpr size_t s_QuadVertexCount{ 4U };
	constexpr CinMath::Vector4 QuadVertexPositions[s_QuadVertexCount]
	{ 
		CinMath::Vector4{  0.5f,  0.5f, 0.0f, 1.0f },
		CinMath::Vector4{  0.5f, -0.5f, 0.0f, 1.0f },
		CinMath::Vector4{ -0.5f, -0.5f, 0.0f, 1.0f },
		CinMath::Vector4{ -0.5f,  0.5f, 0.0f, 1.0f }
	};

	constexpr CinMath::Vector2 QuadVertexTextureCoordinates[s_QuadVertexCount]
	{
		CinMath::Vector2{ 1.0f, 0.0f }, 
		CinMath::Vector2{ 1.0f, 1.0f }, 
		CinMath::Vector2{ 0.0f, 1.0f }, 
		CinMath::Vector2{ 0.0f, 0.0f }
	};

	struct Renderer2DUniformBufferBlock final
	{
		CinMath::Matrix4 CameraViewProjection;
	};

	Renderer2D::Renderer2D(
		const STL::Unique<Renderer>& renderer,
		const STL::Unique<VulkanAllocator>& allocator,
		const STL::Unique<Framebuffer>& targetFramebuffer) noexcept
		:
		m_Renderer(renderer),
		m_Allocator(allocator),
		m_TargetFramebuffer(targetFramebuffer),
		m_RenderCommandBuffer(STL::MakeUnique<RenderCommandBuffer>(m_Renderer->GetDevice(), m_Renderer->GetSwapchain()->GetImageCount())),
		m_QuadIndexBuffer(STL::MakeUnique<IndexBuffer>(m_Allocator, s_MaxQuads * sizeof(uint32_t) * 6U)),
		m_QuadShader(STL::MakeUnique<Shader>(m_Allocator, "Resources/shaders/SimpleShader.shader", false)),
		m_QuadMaterial(STL::MakeUnique<Material>(m_QuadShader)),
		m_QuadPipeline(STL::MakeUnique<Pipeline>(m_Renderer->GetDevice(), m_TargetFramebuffer, m_QuadShader, Geometry::QuadVertex::GetLayout(), EPrimitiveTopology::Triangles)),
		m_UniformBuffers(),
		m_BatchIndex(0U),
		m_DeferredPrepass
		{
			.Layout					{ Geometry::QuadVertex::GetLayout() },
			.Shader					{ nullptr },
			.Pipeline				{ nullptr },
			.Material				{ nullptr },
			.OffscreenFramebuffer	{ nullptr }
		},
		m_DeferredPass
		{
			.Shader					{ nullptr },
			.Pipeline				{ nullptr },
			.Material				{ nullptr },
		},
		m_LightBuffer
		{
			.LightCount				{ 0 },
			.LightBuffer			{ },
			.LightBufferBase		{ nullptr },
		}
	{
		BuildDeferredPrepass();
		BuildDeferredPass();

		uint32_t* const indexData{ cinew uint32_t[s_MaxQuads * 6U] };
		
		uint32_t offset{ 0U };
		for (uint32_t i{ 0 }; i < s_MaxVertices; i += 6U)
		{
			indexData[i + 0U] = offset + 0U;
			indexData[i + 1U] = offset + 1U;
			indexData[i + 2U] = offset + 2U;

			indexData[i + 3U] = offset + 2U;
			indexData[i + 4U] = offset + 3U;
			indexData[i + 5U] = offset + 0U;

			offset += 4U;
		}

		m_QuadIndexBuffer->SetData(indexData, s_MaxIndices * sizeof(uint32_t));
		cindelarr indexData;

		/* Create uniform buffers */
		const uint32_t imageCount{ m_Renderer->GetSwapchain()->GetImageCount() };
		m_UniformBuffers.resize(imageCount);
		for (size_t i{ 0U }; i < imageCount; ++i)
			m_UniformBuffers[i] = STL::MakeUnique<UniformBuffer>(m_Allocator, sizeof(Renderer2DUniformBufferBlock), 0U);

		m_LightUniformBuffers.resize(imageCount);
		for (size_t i{ 0U }; i < imageCount; ++i)
			m_LightUniformBuffers[i] = STL::MakeUnique<UniformBuffer>(m_Allocator, 16016, 0U);

		m_Batches.resize(s_MaxBatches);
		for (Batch& batch : m_Batches)
		{
			batch.QuadVertexBuffer		= STL::MakeUnique<VertexBuffer>(m_Allocator, sizeof(Geometry::QuadVertex) * s_MaxQuads, Geometry::QuadVertex::GetLayout());
			batch.Material				= STL::MakeUnique<Material>(m_DeferredPrepass.Shader);
			
			batch.QuadBufferData		= cinew Geometry::QuadVertex[s_MaxQuads * s_QuadVertexCount];
			batch.QuadBufferDataBase	= &batch.QuadBufferData[0U];
			batch.QuadCount				= 0U;
		}

		m_LightBuffer.LightCount = 0U;
		m_LightBuffer.LightBuffer = cinew Light[s_MaxLights];
		m_LightBuffer.LightBufferBase = &m_LightBuffer.LightBuffer[0U];
	}

	Renderer2D::~Renderer2D() noexcept
	{
		for (const Batch& batch : m_Batches)
			cindelarr batch.QuadBufferDataBase;

		cindelarr m_LightBuffer.LightBufferBase;
	}

	void Renderer2D::BeginFrame(const CinMath::Matrix4& camera)
	{
		m_UniformBuffers[m_Renderer->GetFrameIndex()]->SetData(camera, sizeof(camera));
		m_BatchIndex				= 0U;
		m_FlushCount				= 0U;
		m_LightBuffer.LightCount	= 0U;
		m_LightBuffer.LightBuffer	= m_LightBuffer.LightBufferBase;
	}

	void Renderer2D::EndFrame(const STL::Unique<RenderCommandBuffer>& renderCommandBuffer)
	{
		Flush();
		if (not m_FlushCount)
			return;
		
		const CinMath::Vector3 alignas(16) ambient{ 0.12f, 0.12f, 0.12f };
		const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };

		Byte* mappedData{ m_LightUniformBuffers[frameIndex]->MapData() };
	
		memcpy(mappedData + 0U,		&m_LightBuffer.LightCount,		sizeof(int));
		memcpy(mappedData + 16U,	&ambient,						sizeof(ambient));
		memcpy(mappedData + 32U,	m_LightBuffer.LightBufferBase,	m_LightBuffer.LightCount * sizeof(Light));

		m_LightUniformBuffers[frameIndex]->Unmapdata();

		renderCommandBuffer->Begin(frameIndex);
		m_Renderer->BeginRenderPass(renderCommandBuffer, m_TargetFramebuffer);
				
		const VkDescriptorImageInfo positionSamplerDescriptor
		{
			.sampler{ m_DeferredPrepass.OffscreenFramebuffer->GetSampler() },
			.imageView{ m_DeferredPrepass.OffscreenFramebuffer->GetColorAttachmentView(0U) },
			.imageLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
		};

		m_DeferredPass.Material->SetTexture("u_PositionSampler", positionSamplerDescriptor);
		const VkDescriptorImageInfo albedoSamplerDescriptor
		{
			.sampler{ m_DeferredPrepass.OffscreenFramebuffer->GetSampler() },
			.imageView{ m_DeferredPrepass.OffscreenFramebuffer->GetColorAttachmentView(1U) },
			.imageLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
		};

		m_DeferredPass.Material->SetTexture("u_AlbedoSampler", albedoSamplerDescriptor);

		m_Renderer->RenderFullscreenQuad(
			renderCommandBuffer,
			m_LightUniformBuffers[frameIndex],
			m_DeferredPass.Pipeline,
			m_DeferredPass.Material);

		m_Renderer->EndRenderPass(renderCommandBuffer);
		renderCommandBuffer->End(frameIndex);
		renderCommandBuffer->Submit(frameIndex);
		renderCommandBuffer->Wait(frameIndex);
	}

	void Renderer2D::SetViewportSize(const uint32_t width, const uint32_t height)
	{
		m_DeferredPrepass.OffscreenFramebuffer->Invalidate(width, height);
	}

	void Renderer2D::RenderQuad(const CinMath::Matrix4& transform, const CinMath::Vector4& color, const float tilingFactor, Texture2D* texture)
	{
		CIN_ASSERT(texture);

		const int32_t batchIndex{ RetrieveTextureBatchIndex(texture) };
		if (batchIndex == -1)
		{
			// Flush
			CIN_UNIMPLEMENTED();
		}

		Batch& batch{ m_Batches[batchIndex] };
		for (size_t i{ 0U }; i < s_QuadVertexCount; ++i)
		{
			const CinMath::Vector3 position{ (transform * QuadVertexPositions[i]).xyz };

			batch.QuadBufferData->Position				= position;
			batch.QuadBufferData->Color					= color;
			batch.QuadBufferData->TextureCoordinates	= QuadVertexTextureCoordinates[i];
			batch.QuadBufferData->TilingFactor			= tilingFactor;

			++batch.QuadBufferData;
		}

		++batch.QuadCount;
		//Flush();
	}

	void Renderer2D::RenderLight(const CinMath::Vector3& position, const CinMath::Vector4& color, const float intensity)
	{
		m_LightBuffer.LightBuffer->Position		= position;
		m_LightBuffer.LightBuffer->Color		= color;
		m_LightBuffer.LightBuffer->Intensity	= intensity;
		
		++m_LightBuffer.LightCount;
		++m_LightBuffer.LightBuffer;
	}

	void Renderer2D::BuildDeferredPrepass()
	{
		const auto [width, height] { m_TargetFramebuffer->GetSize() };
		FramebufferSpecification deferredPrepassSpecification
		{
			.Width{ width },
			.Height{ height },
			.Samples{ 1U },
			.ClearOnLoad{ true },
			.AttachmentSpecifications
			{ 
				{ EImageFormat::R32G32B32A32F, CinMath::Vector4{ 0.0f, 0.0f, 0.0f, 0.0f } }, 
				{ EImageFormat::R8G8B8A8, CinMath::Vector4{ 0.0f, 0.0f, 0.0f, 0.0f } } 
			}
		};

		m_DeferredPrepass.OffscreenFramebuffer = STL::MakeUnique<Framebuffer>
		(
			m_Renderer->GetAllocator(), 
			std::move(deferredPrepassSpecification)
		);

		m_DeferredPrepass.Shader = STL::MakeUnique<Shader>
		(
			m_Renderer->GetAllocator(), 
			"Resources/shaders/DeferredPrepass.shader", 
			false
		);

		m_DeferredPrepass.Pipeline = STL::MakeUnique<Pipeline>
		(
			m_Renderer->GetDevice(),
			m_DeferredPrepass.OffscreenFramebuffer,
			m_DeferredPrepass.Shader,
			m_DeferredPrepass.Layout,
			EPrimitiveTopology::Triangles
		);

		m_DeferredPrepass.Material = STL::MakeUnique<Material>(m_DeferredPrepass.Shader);
	}

	void Renderer2D::BuildDeferredPass()
	{
		VertexBufferLayout layout{};

		m_DeferredPass.Shader = STL::MakeUnique<Shader>
		(
			m_Renderer->GetAllocator(), 
			"Resources/shaders/Deferred.shader", 
			false
		);

		m_DeferredPass.Pipeline = STL::MakeUnique<Pipeline>
		(
			m_Renderer->GetDevice(),
			m_TargetFramebuffer,
			m_DeferredPass.Shader,
			layout,
			EPrimitiveTopology::Triangles
		);

		m_DeferredPass.Material = STL::MakeUnique<Material>(m_DeferredPass.Shader);
	}

	void Renderer2D::Flush()
	{
		if (!m_BatchIndex)
			return;
		
		const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };
		m_RenderCommandBuffer->Begin(frameIndex);
		m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_DeferredPrepass.OffscreenFramebuffer);

		for (size_t i{ 0U }; i < m_BatchIndex; ++i)
		{
			Batch& currentBatch{ m_Batches[i] };
			const size_t batchSize{ currentBatch.QuadCount * s_QuadVertexCount * sizeof(Geometry::QuadVertex) };
		
			currentBatch.QuadVertexBuffer->SetData(
				currentBatch.QuadBufferDataBase, batchSize);
		
			m_Renderer->RenderGeometry
			(
				m_RenderCommandBuffer,
				m_UniformBuffers[frameIndex],
				currentBatch.QuadVertexBuffer,
				m_QuadIndexBuffer,
				m_DeferredPrepass.Pipeline,
				currentBatch.Material,
				currentBatch.QuadCount * 6U
			);
		
			/* Reset the batch. */
			currentBatch.QuadCount = 0;
			currentBatch.QuadBufferData = currentBatch.QuadBufferDataBase;
		}
		
		m_Renderer->EndRenderPass(m_RenderCommandBuffer);
		m_RenderCommandBuffer->End(frameIndex);
		m_RenderCommandBuffer->Submit(frameIndex);
		m_RenderCommandBuffer->Wait(frameIndex);

		m_BatchIndex = 0U;
		m_BatchMap.clear();
		++m_FlushCount;
	}

	int32_t Renderer2D::RetrieveTextureBatchIndex(Texture2D* const texture)
	{
		CIN_ASSERT(texture);
		if (m_BatchMap.contains(texture))
			return m_BatchMap[texture];

		if (m_BatchIndex == s_MaxQuads - 1U)
			return -1;

		m_Batches[m_BatchIndex].Material->SetTexture("u_Texture", texture);
		return m_BatchIndex++;
	}
}