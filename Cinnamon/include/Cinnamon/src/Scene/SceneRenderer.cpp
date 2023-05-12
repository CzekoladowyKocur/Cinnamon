#include "Cinnamon/include/Scene/SceneRenderer.hpp"
#include "Cinnamon/include/Renderer/Renderer.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/RenderCommandBuffer.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"
#include "Cinnamon/include/Renderer/IndexBuffer.hpp"
#include "Cinnamon/include/Renderer/Swapchain.hpp"
#include "Cinnamon/include/Renderer/Renderer2D.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

namespace Cinnamon {
	SceneRenderer::SceneRenderer(
		const STL::Unique<Renderer>& renderer,
		const uint32_t viewportWidth,
		const uint32_t viewportHeight) noexcept
		:
		m_Renderer(renderer),
		m_Allocator(STL::MakeUnique<VulkanAllocator>(m_Renderer->GetDevice())),
		m_Framebuffer(STL::MakeUnique<Framebuffer>(m_Allocator, FramebufferSpecification{ viewportWidth, viewportHeight, 1U, EImageFormat::R8G8B8A8 })),
		m_Renderer2D(STL::MakeUnique<Renderer2D>(m_Renderer, m_Allocator, m_Framebuffer))
	{}

	SceneRenderer::~SceneRenderer() noexcept
	{
		/* Resources could still be in used in the previous frames */
		VK_CHECK(vkDeviceWaitIdle(
			m_Renderer->GetDevice()->GetLogicalDevice()));
	}

	void SceneRenderer::BeginFrame()
	{
		m_Renderer2D->BeginFrame();
		m_Renderer2D->RenderQuad(CinMath::Matrix4::Identity());

		CinMath::Matrix4 mat
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.2f, 0.2f, 0.0f, 1.0f,
		};

		m_Renderer2D->RenderQuad(mat);
		m_Renderer2D->EndFrame();
	}

	void SceneRenderer::EndFrame()
	{}

	void SceneRenderer::SetViewportSize(const uint32_t viewportWidth, const uint32_t viewportHeight)
	{
		if (viewportWidth == 0U || viewportHeight == 0U)
			return;
		
		/* Resources could still be in used in the previous frame */
		VK_CHECK(vkDeviceWaitIdle(
			m_Renderer->GetDevice()->GetLogicalDevice()));

		m_Framebuffer->Invalidate(viewportWidth, viewportHeight);
	}

	STL::Unique<Framebuffer>& SceneRenderer::GetFramebuffer() noexcept
	{
		return m_Framebuffer;
	}
}