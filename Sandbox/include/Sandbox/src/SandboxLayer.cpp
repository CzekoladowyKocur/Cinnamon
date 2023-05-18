#include "Sandbox/include/SandboxLayer.hpp"
#include "Cinnamon/include/Core/Logger.hpp"
#include "Cinnamon/include/Event/WindowEvent.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/Material.hpp"
#include "Cinnamon/include/GUI/GUI.hpp"

#include "ThirdParty/imgui/imgui.h"

using namespace Cinnamon;
SandboxLayer::SandboxLayer(STL::Unique<Window>& window, STL::Unique<Renderer>& renderer) noexcept
	:
	m_Window(window),
	m_Renderer(renderer),
	m_Allocator(cinew VulkanAllocator(m_Renderer->GetDevice())),
	m_Framebuffer(STL::MakeUnique<Framebuffer>(m_Allocator, FramebufferSpecification{ m_Window->GetWidth(), m_Window->GetHeight(), 1U, EImageFormat::R8G8B8A8 }, m_Renderer->GetSwapchain())),
	m_RenderCommandBuffer(STL::MakeUnique<RenderCommandBuffer>(m_Renderer->GetDevice(), m_Renderer->GetSwapchain()))
{
#if 1
	m_Window->SetEventCallback([](const Event& /*event*/) 
	{});

	m_Renderer->SetClearColor(0.3f, 0.3f, 1.0f, 1.0f);

	VertexBufferLayout layout
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
	m_QuadShader = (STL::MakeUnique<Shader>(m_Allocator, "Resources/shaders/BasicQuad.shader", true));
	m_Pipeline = (STL::MakeUnique<Pipeline>(m_Renderer->GetDevice(), m_Framebuffer, m_QuadShader, 
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
#endif
}

SandboxLayer::~SandboxLayer() noexcept
{}

void SandboxLayer::OnAttach()
{
}

void SandboxLayer::OnUpdate(const Timestep /*timestep*/)
{
	const uint32_t frameIndex{ m_Renderer->GetFrameIndex() };
	m_RenderCommandBuffer->Begin(frameIndex);
	
	m_Renderer->BeginRenderPass(m_RenderCommandBuffer, m_Framebuffer);
	
	m_Renderer->RenderGeometry
	(
		m_RenderCommandBuffer,
		m_QuadVertexBuffer,
		m_QuadIndexBuffer,
		m_Pipeline,
		6U
	);
	
	m_Renderer->EndRenderPass(m_RenderCommandBuffer);
	m_RenderCommandBuffer->End(frameIndex);
	m_RenderCommandBuffer->Submit(frameIndex);
	m_RenderCommandBuffer->Wait(frameIndex);
}

void SandboxLayer::OnDetach()
{}

void SandboxLayer::OnEvent(const Event& /*event*/)
{}