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

	constexpr CinMath::Vector4 LineVertexPositions[4U]
	{ 
		CinMath::Vector4{ -0.5f, -0.5f, 0.0f, 1.0f }, 
		CinMath::Vector4{  0.5f, -0.5f, 0.0f, 1.0f }, 
		CinMath::Vector4{  0.5f,  0.5f, 0.0f, 1.0f }, 
		CinMath::Vector4{ -0.5f,  0.5f, 0.0f, 1.0f } 
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
		m_QuadPipeline(STL::MakeUnique<Pipeline>(m_Renderer->GetDevice(), m_TargetFramebuffer, m_QuadShader, Geometry::QuadVertex::GetLayout(), EPrimitiveTopology::Triangles)),
		m_WhiteTexture(nullptr),
		m_UniformBuffers(),
		m_BatchIndex(0U),
		m_Deferred
		{
			.PrepassLayout					{ Geometry::QuadVertex::GetLayout() },
			.PrepassShader					{ nullptr },
			.PrepassPipeline				{ nullptr },
			.PrepassMaterial				{ nullptr },
			.PrepassOffscreenFramebuffer	{ nullptr },
			.PassShader						{ nullptr },
			.PassPipeline					{ nullptr },
			.PassMaterial					{ nullptr },
		},
		m_LightBuffer
		{
			.LightCount				{ 0 },
			.Padding				{ 0, 0, 0 },
			.LightBuffer			{ },
			.LightBufferBase		{ nullptr },
		},
		m_AmbientLight(0.15f),
		m_LightUniformBuffers()
	{
		BuildDeferredPrepass();
		BuildDeferredPass();
		BuildLinePipeline();

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
			batch.QuadMaterial			= STL::MakeUnique<Material>(m_Deferred.PrepassShader);
			
			batch.QuadBufferData		= cinew Geometry::QuadVertex[s_MaxQuads * s_QuadVertexCount];
			batch.QuadBufferDataBase	= &batch.QuadBufferData[0U];
			batch.QuadCount				= 0U;
		}

		m_LightBuffer.LightCount = 0U;
		m_LightBuffer.LightBuffer = cinew Light[s_MaxLights];
		m_LightBuffer.LightBufferBase = &m_LightBuffer.LightBuffer[0U];
	
		/* Create a white texture from data */
		{
			Byte* const whiteTextureData{ cinew Byte[s_WhiteTextureSize] };
			memset(whiteTextureData, 0xff'ff'ff'ff, s_WhiteTextureSize);

			m_WhiteTexture = STL::MakeUnique<Texture2D>
			(
				m_Renderer->GetAllocator(),
				whiteTextureData,
				s_WhiteTextureWidth,
				s_WhiteTextureHeight,
				EImageFormat::R8G8B8A8,
				TextureSpecification{}
			);

			cindelarr whiteTextureData;
		}
	}

	Renderer2D::~Renderer2D() noexcept
	{
		for (const Batch& batch : m_Batches)
			cindelarr batch.QuadBufferDataBase;

		cindelarr m_LightBuffer.LightBufferBase;
		cindelarr m_LineBuffer.LineDataBase;
	}

	void Renderer2D::BeginFrame(const CinMath::Matrix4& camera, const CinMath::Vector3& viewPosition, const CinMath::Vector3& ambientLight)
	{
		m_UniformBuffers[m_Renderer->GetFrameIndex()]->SetData(camera, sizeof(camera));
		m_Batches[0U].QuadMaterial->SetTexture("u_Texture", m_WhiteTexture.get());
		
		/* Set null to a white texture. */
		m_BatchMap[m_WhiteTexture.get()]	= 0U;
		m_BatchIndex						= 1U;
		m_FlushCount						= 0U;
		m_LightBuffer.LightCount			= 0U;
		m_LightBuffer.LightBuffer			= m_LightBuffer.LightBufferBase;
		m_CameraViewPosition				= viewPosition;
		m_AmbientLight						= ambientLight;
		m_FramebufferCleared				= false;

		m_LineBuffer.LineCount = 0;
		m_LineBuffer.LineData = m_LineBuffer.LineDataBase;
	}

	void Renderer2D::EndFrame(const STL::Unique<RenderCommandBuffer>& renderCommandBuffer)
	{
		Flush();

		const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };
		if (not m_FlushCount)
		{
			/* Clear pass */
			renderCommandBuffer->Begin(frameIndex);
			m_Renderer->BeginRenderPass(renderCommandBuffer, m_TargetFramebuffer);
			m_Renderer->Clear(renderCommandBuffer, m_TargetFramebuffer);
			m_Renderer->EndRenderPass(renderCommandBuffer);
			renderCommandBuffer->End(frameIndex);
			renderCommandBuffer->Submit(frameIndex);
			renderCommandBuffer->Wait(frameIndex);

			return;
		}
		
		Byte* mappedData{ m_LightUniformBuffers[frameIndex]->MapData() };
	
		memcpy(mappedData + 0U,		&m_LightBuffer.LightCount,		sizeof(m_LightBuffer.LightCount));
		memcpy(mappedData + 16U,	&m_AmbientLight,				sizeof(m_AmbientLight));
		memcpy(mappedData + 32U,	&m_CameraViewPosition,			sizeof(m_CameraViewPosition));
		memcpy(mappedData + 48U,	m_LightBuffer.LightBufferBase,	m_LightBuffer.LightCount * sizeof(Light));

		m_LightUniformBuffers[frameIndex]->Unmapdata();

		renderCommandBuffer->Begin(frameIndex);
		m_Renderer->BeginRenderPass(renderCommandBuffer, m_TargetFramebuffer);
				
		const VkDescriptorImageInfo positionSamplerDescriptor
		{
			.sampler{ m_Deferred.PrepassOffscreenFramebuffer->GetSampler() },
			.imageView{ m_Deferred.PrepassOffscreenFramebuffer->GetColorAttachmentView(0U) },
			.imageLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
		};

		m_Deferred.PassMaterial->SetTexture("u_PositionSampler", positionSamplerDescriptor);
		const VkDescriptorImageInfo albedoSamplerDescriptor
		{
			.sampler{ m_Deferred.PrepassOffscreenFramebuffer->GetSampler() },
			.imageView{ m_Deferred.PrepassOffscreenFramebuffer->GetColorAttachmentView(1U) },
			.imageLayout{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
		};

		m_Deferred.PassMaterial->SetTexture("u_AlbedoSampler", albedoSamplerDescriptor);

		m_Renderer->RenderFullscreenQuad(
			renderCommandBuffer,
			m_LightUniformBuffers[frameIndex],
			m_Deferred.PassPipeline,
			m_Deferred.PassMaterial);

		if (m_LineBuffer.LineCount)
		{
			const size_t lineBufferSize{ m_LineBuffer.LineCount * 2U * sizeof(Geometry::LineVertex) };
			m_LinePipeline.VertexBuffer_->SetData(m_LineBuffer.LineDataBase, lineBufferSize);

			m_Renderer->RenderGeometry
			(
				renderCommandBuffer,
				m_UniformBuffers[frameIndex],
				m_LinePipeline.VertexBuffer_,
				m_LinePipeline.IndexBuffer_,
				m_LinePipeline.Pipeline_,
				m_LinePipeline.Material_,
				(uint32_t)m_LineBuffer.LineCount * 2U
			);
		}

		m_Renderer->EndRenderPass(renderCommandBuffer);
		renderCommandBuffer->End(frameIndex);
		renderCommandBuffer->Submit(frameIndex);
		renderCommandBuffer->Wait(frameIndex);
	}

	void Renderer2D::SetViewportSize(const uint32_t width, const uint32_t height)
	{
		m_Deferred.PrepassOffscreenFramebuffer->Invalidate(width, height);
	}

	void Renderer2D::RenderQuad(const CinMath::Matrix4& transform, const CinMath::Vector4& color)
	{
		const int32_t batchIndex{ RetrieveTextureBatchIndex(m_WhiteTexture.get()) };
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
			batch.QuadBufferData->TilingFactor			= 1.0f;

			++batch.QuadBufferData;
		}

		++batch.QuadCount;
		Flush();
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
		Flush();
	}

	void Renderer2D::RenderLine(const CinMath::Vector3& positionA, const CinMath::Vector3& positionB, const CinMath::Vector4& color)
	{
		m_LineBuffer.LineData->Position = positionA;
		m_LineBuffer.LineData->Color	= color;
		++m_LineBuffer.LineData;

		m_LineBuffer.LineData->Position = positionB;
		m_LineBuffer.LineData->Color	= color;
		++m_LineBuffer.LineData;
		
		++m_LineBuffer.LineCount;
	}

	void Renderer2D::RenderAABB(const CinMath::Matrix4& transform, const CinMath::Vector4 color)
	{
		RenderLine((transform * LineVertexPositions[0]).xyz, (transform * LineVertexPositions[1]).xyz, color);
		RenderLine((transform * LineVertexPositions[1]).xyz, (transform * LineVertexPositions[2]).xyz, color);
		RenderLine((transform * LineVertexPositions[2]).xyz, (transform * LineVertexPositions[3]).xyz, color);
		RenderLine((transform * LineVertexPositions[3]).xyz, (transform * LineVertexPositions[0]).xyz, color);
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
			.ClearOnLoad{ false },
			.AttachmentSpecifications
			{ 
				{ EImageFormat::R32G32B32A32F, CinMath::Vector4{ 0.0f, 0.0f, 0.0f, 0.0f } }, 
				{ EImageFormat::R8G8B8A8, CinMath::Vector4{ 0.0f, 0.0f, 0.0f, 0.0f } } 
			}
		};

		m_Deferred.PrepassOffscreenFramebuffer = STL::MakeUnique<Framebuffer>
		(
			m_Renderer->GetAllocator(), 
			std::move(deferredPrepassSpecification)
		);

		m_Deferred.PrepassShader = STL::MakeUnique<Shader>
		(
			m_Renderer->GetAllocator(), 
			"Resources/shaders/DeferredPrepass.shader", 
			false
		);

		m_Deferred.PrepassPipeline = STL::MakeUnique<Pipeline>
		(
			m_Renderer->GetDevice(),
			m_Deferred.PrepassOffscreenFramebuffer,
			m_Deferred.PrepassShader,
			m_Deferred.PrepassLayout,
			EPrimitiveTopology::Triangles
		);

		m_Deferred.PrepassMaterial = STL::MakeUnique<Material>(m_Deferred.PrepassShader);
	}

	void Renderer2D::BuildDeferredPass()
	{
		VertexBufferLayout layout{};

		m_Deferred.PassShader = STL::MakeUnique<Shader>
		(
			m_Renderer->GetAllocator(), 
			"Resources/shaders/Deferred.shader", 
			false
		);

		m_Deferred.PassPipeline = STL::MakeUnique<Pipeline>
		(
			m_Renderer->GetDevice(),
			m_TargetFramebuffer,
			m_Deferred.PassShader,
			layout,
			EPrimitiveTopology::Triangles
		);

		m_Deferred.PassMaterial = STL::MakeUnique<Material>(m_Deferred.PassShader);
	}

	void Renderer2D::BuildLinePipeline()
	{
		m_LinePipeline.VertexBuffer_	= STL::MakeUnique<VertexBuffer>(m_Allocator, 1000U, Geometry::LineVertex::GetLayout());
		m_LinePipeline.IndexBuffer_		= STL::MakeUnique<IndexBuffer>(m_Allocator, 1000U);
		m_LinePipeline.Shader_			= STL::MakeUnique<Shader>(m_Allocator, "Resources/shaders/Line.shader", false);
		m_LinePipeline.Material_		= STL::MakeUnique<Material>(m_LinePipeline.Shader_);
		m_LinePipeline.Pipeline_		= STL::MakeUnique<Pipeline>(m_Renderer->GetDevice(), m_TargetFramebuffer, m_LinePipeline.Shader_, Geometry::LineVertex::GetLayout(), EPrimitiveTopology::Lines);

		m_LineBuffer.LineData			= cinew Geometry::LineVertex[100U];
		m_LineBuffer.LineDataBase		= &m_LineBuffer.LineData[0U];
		m_LineBuffer.LineCount			= 0U;

		uint32_t* lineIndices{ cinew uint32_t[100] };
		for (uint32_t i = 0; i < 100; ++i)
			lineIndices[i] = i;

		m_LinePipeline.IndexBuffer_->SetData(lineIndices, sizeof(uint32_t) * 100U);
		cindelarr lineIndices;
	}

	void Renderer2D::Flush()
	{
		if (m_BatchIndex == 1U and not m_Batches[m_BatchIndex - 1U].QuadCount)
			return;
		
		const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };
		m_RenderCommandBuffer->Begin(frameIndex);
		m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_Deferred.PrepassOffscreenFramebuffer);

		if (not m_FramebufferCleared)
		{
			m_Renderer->Clear(m_RenderCommandBuffer, m_Deferred.PrepassOffscreenFramebuffer);
			m_FramebufferCleared = true;
		}

		for (size_t i{ 0U }; i < m_BatchIndex; ++i)
		{
			Batch& currentBatch{ m_Batches[i] };
			
			if (not currentBatch.QuadCount)
				continue;
		
			const size_t batchSize{ currentBatch.QuadCount * s_QuadVertexCount * sizeof(Geometry::QuadVertex) };
			currentBatch.QuadVertexBuffer->SetData(
				currentBatch.QuadBufferDataBase, batchSize);
		
			m_Renderer->RenderGeometry
			(
				m_RenderCommandBuffer,
				m_UniformBuffers[frameIndex],
				currentBatch.QuadVertexBuffer,
				m_QuadIndexBuffer,
				m_Deferred.PrepassPipeline,
				currentBatch.QuadMaterial,
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

		m_BatchIndex = 1U;
		m_BatchMap.clear();
		++m_FlushCount;
	}

	int32_t Renderer2D::RetrieveTextureBatchIndex(Texture2D* const texture)
	{
		if (m_BatchMap.contains(texture))
			return m_BatchMap[texture];

		if (m_BatchIndex == s_MaxQuads - 1U)
			return -1;

		m_Batches[m_BatchIndex].QuadMaterial->SetTexture("u_Texture", texture);
		return m_BatchIndex++;
	}
}