#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Renderer/Geometry.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class Renderer;
	class VulkanAllocator;
	class Framebuffer;
	class RenderCommandBuffer;
	class VertexBuffer;
	class IndexBuffer;
	class Shader;
	class Pipeline;
	// Resources
	class Texture2D;

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

		void RenderQuad(const CinMath::Matrix4& transform);
		void RenderQuad(const CinMath::Matrix4& transform, const Texture2D& texture);
	private:
		const STL::Unique<Renderer>&			m_Renderer;
		const STL::Unique<VulkanAllocator>&		m_Allocator;
		const STL::Unique<Framebuffer>&			m_TargetFramebuffer;

		STL::Unique<RenderCommandBuffer>		m_RenderCommandBuffer;
		STL::Unique<VertexBuffer>				m_QuadVertexBuffer;
		STL::Unique<IndexBuffer>				m_QuadIndexBuffer;
		STL::Unique<Shader>						m_QuadShader;
		STL::Unique<Pipeline>					m_QuadPipeline;

		Geometry::QuadVertex*					m_QuadBufferData;
		Geometry::QuadVertex*					m_QuadBufferDataBase;

		uint32_t*								m_QuadIndexBufferData;
		uint32_t*								m_QuadIndexBufferDataBase;

		uint32_t								m_QuadCount;

		static inline constexpr size_t			s_MaxQuads{ 1000U };

		/* TODO: Remove, temporary */
		Texture2D* m_BasicTexture;
	};
}