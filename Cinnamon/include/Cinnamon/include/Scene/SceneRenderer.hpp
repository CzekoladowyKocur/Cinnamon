#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class Scene;
	class Renderer;
	class Renderer2D; 
	class Framebuffer;
	class RenderCommandBuffer;	
	class VulkanAllocator;
	class VertexBuffer;
	class IndexBuffer;
	class Pipeline;
	class Shader;
	class Material;
	class Texture2D;

	class SceneRenderer final
	{
	private:
		NON_COPYABLE(SceneRenderer)
	public:
		explicit SceneRenderer(
			const STL::Unique<Renderer>& renderer,
			const bool swapchainTarget,
			const uint32_t viewportWidth,
			const uint32_t viewportHeight) noexcept;

		~SceneRenderer() noexcept;

		void OnUpdate(const Timestep timestep);
		void RenderScene(const CinMath::Matrix4& camera, const CinMath::Vector3& viewPosition);
		void SetRenderedScene(Scene* const scene);
		void SetAspectRatio(const float aspectRatio);
		void SetViewportSize(const uint32_t viewportWidth, const uint32_t viewportHeight);

		[[nodiscard]] STL::Unique<Framebuffer>& 
			GetFramebuffer() noexcept;
	private:
		const STL::Unique<Renderer>& m_Renderer;
		const bool m_SwapchainTarget;
	
		STL::Unique<Framebuffer> m_Framebuffer;
		STL::Unique<Renderer2D> m_Renderer2D;
		STL::Unique<RenderCommandBuffer> m_RenderCommandBuffer;
		STL::Unique<Shader>	m_FullScreenQuadShader;
		STL::Unique<Material> m_SkyBoxMaterial;
		STL::Unique<Pipeline> m_SkyboxPipeline;

		Scene* m_RenderedScene;
		float m_AspectRatio;

		//Texture2D* m_Texture;
	};
}