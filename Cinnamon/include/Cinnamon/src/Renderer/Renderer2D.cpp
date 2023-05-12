#include "Cinnamon/include/Renderer/Renderer2D.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
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
		CinMath::Vector2{ 0.0f, 0.0f }, 
		CinMath::Vector2{ 1.0f, 0.0f }, 
		CinMath::Vector2{ 1.0f, 1.0f }, 
		CinMath::Vector2{ 0.0f, 1.0f }
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
		m_QuadVertexBuffer(STL::MakeUnique<VertexBuffer>(m_Allocator, sizeof(Geometry::QuadVertex)* s_MaxQuads, Geometry::QuadVertex::GetLayout())),
		m_QuadIndexBuffer(STL::MakeUnique<IndexBuffer>(m_Allocator, s_MaxQuads * sizeof(uint32_t) * 6U)),
		m_QuadShader(STL::MakeUnique<Shader>(m_Allocator, "Resources/shaders/SimpleShader.shader", true)),
		m_QuadPipeline(STL::MakeUnique<Pipeline>(m_Renderer->GetDevice(), m_TargetFramebuffer, m_QuadShader, Geometry::QuadVertex::GetLayout(), EPrimitiveTopology::Triangles)),
		m_QuadBufferData(nullptr),
		m_QuadBufferDataBase(nullptr),
		m_QuadIndexBufferData(nullptr),
		m_QuadIndexBufferDataBase(nullptr),
		m_QuadCount(0U),
		m_BasicTexture(cinew Texture2D(m_Allocator, "Resources/textures/paper.png", TextureSpecification{ .SamplerFilterMode{ ETextureSamplerFilterMode::Linear } }))
	{
		/* Allocate buffer space */
		m_QuadBufferData = cinew Geometry::QuadVertex[s_MaxQuads * s_QuadVertexCount];
		m_QuadBufferDataBase = &m_QuadBufferData[0U];

		m_QuadIndexBufferData = cinew uint32_t[s_MaxQuads * 6U];
		m_QuadIndexBufferDataBase = &m_QuadIndexBufferData[0U];
	}

	Renderer2D::~Renderer2D() noexcept
	{
		[[likely]]
		if (m_QuadBufferDataBase)
			cindelarr m_QuadBufferDataBase;
		
		[[likely]]
		if(m_QuadIndexBufferDataBase)
			cindelarr m_QuadIndexBufferDataBase;

		[[likely]]
		if (m_BasicTexture)
			cindel m_BasicTexture;
	}

	void Renderer2D::BeginFrame()
	{
		m_QuadCount = 0U;
		m_QuadBufferData = m_QuadBufferDataBase;
		m_QuadIndexBufferData = m_QuadIndexBufferDataBase;
	}

	void Renderer2D::EndFrame()
	{
		if (m_QuadCount)
		{
			m_QuadVertexBuffer->SetData(m_QuadBufferDataBase, m_QuadCount * s_QuadVertexCount * sizeof(Geometry::QuadVertex));
			m_QuadIndexBuffer->SetData(m_QuadIndexBufferDataBase, m_QuadCount * sizeof(uint32_t) * 6U);
			
			const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };
			m_RenderCommandBuffer->Begin(frameIndex);
			
			m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_TargetFramebuffer);
			{
				m_Renderer->RenderGeometry
				(
					m_RenderCommandBuffer,
					m_QuadVertexBuffer,
					m_QuadIndexBuffer,
					m_QuadPipeline,
					m_QuadShader,
					*m_BasicTexture,
					m_QuadCount * 6U
				);
			}
			m_Renderer->EndRenderPass(m_RenderCommandBuffer);
			
			m_RenderCommandBuffer->End(frameIndex);
			m_RenderCommandBuffer->Submit(frameIndex);
			m_RenderCommandBuffer->Wait(frameIndex);
		}
	}

	void Renderer2D::RenderQuad(const CinMath::Matrix4& transform)
	{
		if (m_QuadCount == s_MaxQuads)
		{
			CIN_WARN("Reached maximum quad count in renderer2D");
			return;
		}

		for (size_t i{ 0U }; i < s_QuadVertexCount; ++i)
		{
			m_QuadBufferData->Position = (transform * QuadVertexPositions[i]).xyz;
			m_QuadBufferData->TextureCoordinates = QuadVertexTextureCoordinates[i];
			
			++m_QuadBufferData;
		}

		uint32_t offset{ m_QuadCount * 4U };
		uint32_t quadIndices[6U]
		{
			0U + offset, 1U + offset, 3U + offset,
			1U + offset, 2U + offset, 3U + offset
		};

		memcpy(m_QuadIndexBufferData, quadIndices, sizeof(quadIndices));
		m_QuadIndexBufferData += 6U;
		
		++m_QuadCount;
	}

	void Renderer2D::RenderQuad(const CinMath::Matrix4& /*transform*/, const Texture2D& /*texture*/)
	{

	}
}