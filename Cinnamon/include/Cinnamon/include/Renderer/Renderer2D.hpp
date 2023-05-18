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
	class Material;
	class Pipeline;
	class UniformBuffer;
	// Resources
	class Texture2D;

	class Renderer2D final
	{
	private:
		struct Batch
		{
			STL::Unique<VertexBuffer>	QuadVertexBuffer;
			STL::Unique<Material>		QuadMaterial;

			Geometry::QuadVertex*		QuadBufferData;
			Geometry::QuadVertex*		QuadBufferDataBase;
			uint32_t					QuadCount;
		};

		struct Light
		{
			constexpr Light() noexcept
				:
				Color(),
				Position(),
				Intensity(1.0f)
			{}

			CinMath::Vector4 Color;
			CinMath::Vector3 Position;
			float Intensity;
		};

		NON_COPYABLE(Renderer2D)
	public:
		explicit Renderer2D(
			const STL::Unique<Renderer>&		renderer,
			const STL::Unique<VulkanAllocator>& allocator,
			const STL::Unique<Framebuffer>&		targetFramebuffer) noexcept;
		
		~Renderer2D() noexcept;
		
		void BeginFrame(const CinMath::Matrix4& camera);
		void EndFrame(const STL::Unique<RenderCommandBuffer>& renderCommandBuffer);
		void SetViewportSize(const uint32_t width, const uint32_t height);

		void RenderQuad(const CinMath::Matrix4& transform, const CinMath::Vector4& color, const float tilingFactor, Texture2D* texture);
		void RenderLight(const CinMath::Vector3& position, const CinMath::Vector4& color, const float intensity);
	private:
		void BuildDeferredPrepass();
		void BuildDeferredPass();
		void Flush();
		int32_t RetrieveTextureBatchIndex(Texture2D* const texture);
	private:
		const STL::Unique<Renderer>&			m_Renderer;
		const STL::Unique<VulkanAllocator>&		m_Allocator;
		const STL::Unique<Framebuffer>&			m_TargetFramebuffer;

		STL::Unique<RenderCommandBuffer>		m_RenderCommandBuffer;
		STL::Unique<IndexBuffer>				m_QuadIndexBuffer;
		STL::Unique<Shader>						m_QuadShader;
		STL::Unique<Pipeline>					m_QuadPipeline;
		STL::Vector<STL::Unique<UniformBuffer>>	m_UniformBuffers;
				
		STL::UMap<Texture2D*, int32_t>			m_BatchMap;
		STL::Vector<Batch>						m_Batches;
		uint32_t								m_BatchIndex;
		uint32_t								m_FlushCount;

		struct
		{
			/* Deferred prepass */
			VertexBufferLayout			PrepassLayout;
			STL::Unique<Shader>			PrepassShader;
			STL::Unique<Pipeline>		PrepassPipeline;
			STL::Unique<Material>		PrepassMaterial;
			STL::Unique<Framebuffer>	PrepassOffscreenFramebuffer;

			/* Deferred pass */
			STL::Unique<Shader>			PassShader;
			STL::Unique<Pipeline>		PassPipeline;
			STL::Unique<Material>		PassMaterial;
		} m_Deferred;

		struct
		{
			int32_t	LightCount;
			int32_t Padding[3];

			Light*	LightBuffer;
			Light*	LightBufferBase;
		} m_LightBuffer;

		STL::Vector<STL::Unique<UniformBuffer>>	m_LightUniformBuffers;

		static inline constexpr size_t			s_MaxQuads{ 100U };
		static inline constexpr size_t			s_MaxVertices{ s_MaxQuads * 4U };
		static inline constexpr size_t			s_MaxIndices{ s_MaxQuads * 6U };
		static inline constexpr size_t			s_MaxBatches{ 32U };
		static inline constexpr size_t			s_MaxLights{ 100U };
	};
}