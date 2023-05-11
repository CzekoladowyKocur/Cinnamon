#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	class VulkanAllocator;

	enum class EShaderType
	{
		None,
		Vertex,
		Fragment,
		Compute,
	};

	constexpr const char* ShaderTypeToString(const EShaderType shaderType) noexcept
	{
		switch (shaderType)
		{
			case EShaderType::None:		return "None";
			case EShaderType::Vertex:	return "Vertex";
			case EShaderType::Fragment: return "Fragment";
			case EShaderType::Compute:	return "Compute";
			
			[[unlikely]]
			default: CIN_ASSERT(false); return "Unknown shader";
		}
	}

	class Shader final
	{
	private:
		NON_COPYABLE(Shader)
	public:
		explicit Shader(
			const STL::Unique<VulkanAllocator>& allocator,
			const STL::Filepath& filepath, 
			const bool forceCompile) noexcept;

		~Shader() noexcept;

		void Reflect();

		[[nodiscard]] const STL::Vector<VkDescriptorSetLayout>& 
			GetDescriptorSetLayouts() const;
		
		[[nodiscard]] const STL::Vector<VkPipelineShaderStageCreateInfo>&
			GetPipelineStages() const;
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;

		STL::Vector<VkPipelineShaderStageCreateInfo> m_PipelineStages;
		STL::Vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;


		STL::UMap<EShaderType, STL::String> m_ShaderSources;
		STL::UMap<EShaderType, STL::Vector<uint32_t>> m_ShaderBinaries;
	};
}