#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	class Framebuffer;
	class Shader;
	class Device;
	struct VertexBufferLayout;
	
	enum class EPrimitiveTopology
	{
		Points		= VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		Lines		= VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		Triangles	= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	class Pipeline final
	{
	private:
		NON_COPYABLE(Pipeline)
	public:
		explicit Pipeline(
			const STL::Unique<Device>& device,
			const STL::Unique<Framebuffer>& framebuffer,
			const STL::Unique<Shader>& shader,
			const VertexBufferLayout& vertexBufferLayout,
			const EPrimitiveTopology topology) noexcept;

		~Pipeline() noexcept;

		void Invalidate();

		[[nodiscard]] VkPipeline
			GetHandle() const;

		[[nodiscard]] VkPipelineLayout
			GetLayout() const;
	private:
		const STL::Unique<Device>& m_Device;
		const STL::Unique<Framebuffer>& m_Framebuffer;
		const STL::Unique<Shader>& m_Shader;
		const VertexBufferLayout& m_VertexBufferLayout;
		const EPrimitiveTopology m_Topology;

		VkPipeline m_Handle;
		VkPipelineCache m_PipelineCache;
		VkPipelineLayout m_Layout;
	};
}