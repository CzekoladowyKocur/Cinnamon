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

namespace Cinnamon {
	Renderer2D::Renderer2D(
		const STL::Unique<Renderer>& renderer, 
		const STL::Unique<VulkanAllocator>& allocator,
		const STL::Unique<Framebuffer>& targetFramebuffer) noexcept
		:
		m_Renderer(renderer),
		m_Allocator(allocator),
		m_TargetFramebuffer(targetFramebuffer),
		m_RenderCommandBuffer(STL::MakeUnique<RenderCommandBuffer>(m_Renderer->GetDevice(), m_Renderer->GetSwapchain()->GetImageCount()))
	{
		const VertexBufferLayout layout
		(
			STL::InitializerList<VertexBufferElement>
			{
				VertexBufferElement
				{
					EShaderDataType::Float3
				}
			}
		);

		m_QuadVertexBuffer = (STL::MakeUnique<VertexBuffer>(m_Allocator, sizeof(float) * 3U * 4U, layout));
		m_QuadIndexBuffer = (STL::MakeUnique<IndexBuffer>(m_Allocator, sizeof(uint32_t) * 6U));
		m_QuadShader = (STL::MakeUnique<Shader>(m_Allocator, "Resources/shaders/SimpleShader.shader", false));
		m_QuadPipeline = (STL::MakeUnique<Pipeline>(m_Renderer->GetDevice(), m_TargetFramebuffer, m_QuadShader,
			m_QuadVertexBuffer->GetLayout(), EPrimitiveTopology::Triangles));

		float vertices[3 * 4]
		{
			0.5f,  0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
		   -0.5f, -0.5f, 0.0f,
		   -0.5f,  0.5f, 0.0f
		};

		m_QuadVertexBuffer->SetData(vertices, sizeof(vertices));

		uint32_t indices[6]
		{
			0, 1, 3,
			1, 2, 3
		};

		m_QuadIndexBuffer->SetData(indices, sizeof(indices));
	}

	Renderer2D::~Renderer2D() noexcept
	{}

	void Renderer2D::BeginFrame()
	{}

	void Renderer2D::EndFrame()
	{
		const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };
		m_RenderCommandBuffer->Begin(frameIndex);

		m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_TargetFramebuffer);

		m_Renderer->RenderGeometry
		(
			m_RenderCommandBuffer,
			m_QuadVertexBuffer,
			m_QuadIndexBuffer,
			m_QuadPipeline,
			6U
		);

		m_Renderer->EndRenderPass(m_RenderCommandBuffer);

		m_RenderCommandBuffer->End(frameIndex);
		m_RenderCommandBuffer->Submit(frameIndex);
		m_RenderCommandBuffer->Wait(frameIndex);
	}
}