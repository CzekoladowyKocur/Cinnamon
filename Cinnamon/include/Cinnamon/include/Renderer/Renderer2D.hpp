#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Renderer;
	class VulkanAllocator;
	class Framebuffer;
	class RenderCommandBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class Shader;
	class Pipeline;

	class Renderer2D final
	{
	private:
		NON_COPYABLE(Renderer2D)
	public:
		explicit Renderer2D(
			const STL::Unique<Renderer>& renderer,
			const STL::Unique<VulkanAllocator>& allocator,
			const STL::Unique<Framebuffer>& targetFramebuffer) noexcept;
		
		~Renderer2D() noexcept;
		
		void BeginFrame();
		void EndFrame();
	private:
		const STL::Unique<Renderer>&			m_Renderer;
		const STL::Unique<VulkanAllocator>&		m_Allocator;
		const STL::Unique<Framebuffer>&			m_TargetFramebuffer;

		STL::Unique<RenderCommandBuffer>		m_RenderCommandBuffer;
		STL::Unique<VertexBuffer>				m_QuadVertexBuffer;
		STL::Unique<IndexBuffer>				m_QuadIndexBuffer;
		STL::Unique<Shader>						m_QuadShader;
		STL::Unique<Pipeline>					m_QuadPipeline;
	};
}