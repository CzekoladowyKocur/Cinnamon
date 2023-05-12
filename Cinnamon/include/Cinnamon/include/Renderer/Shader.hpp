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

	struct ImageSamplerDescription
	{
		std::string Name;
		uint32_t BindingPoint;
		uint32_t DescriptorSet;
		uint32_t ArraySize;

		VkShaderStageFlagBits ShaderStage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
	};

	struct ShaderDescriptorSet final
	{
		STL::UMap<uint32_t, ImageSamplerDescription>	ImageSamplers;
		STL::UMap<STL::String, VkWriteDescriptorSet>	WriteDescriptorSets;
		STL::Vector<VkDescriptorPoolSize>				DescriptorPoolSizes;
	};

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
		
		[[nodiscard]] VkDescriptorSet 
			AllocateDescriptorSet(const uint32_t set, const VkDescriptorPool descriptorPool);

		[[nodiscard]] const STL::Vector<VkDescriptorSetLayout>& 
			GetDescriptorSetLayouts() const;
		
		[[nodiscard]] const STL::Vector<VkPipelineShaderStageCreateInfo>&
			GetPipelineStages() const;
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;

		STL::Vector<VkPipelineShaderStageCreateInfo> m_PipelineStages;
		STL::Vector<VkDescriptorSetLayout> m_ShaderDescriptorSetLayouts;
		STL::Vector<ShaderDescriptorSet> m_ShaderDescriptorSets;

		STL::UMap<EShaderType, STL::String> m_ShaderSources;
		STL::UMap<EShaderType, STL::Vector<uint32_t>> m_ShaderBinaries;

		STL::Vector<STL::Vector<VkDescriptorSet>> m_DescriptorSetHandles;
	};
}