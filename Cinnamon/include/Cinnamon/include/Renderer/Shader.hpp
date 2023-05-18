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

	struct ImageSamplerDescription final
	{
		STL::String Name;
		uint32_t BindingPoint;
		uint32_t DescriptorSet;
		uint32_t ArraySize;

		VkShaderStageFlags ShaderStage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
	};

	struct UniformBufferDescription final
	{
		STL::String Name;
		uint32_t BindingPoint;
		uint32_t DescriptorSet;

		VkShaderStageFlags ShaderStage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };
	};

	struct ShaderDescriptorSet final
	{
		STL::UMap<uint32_t, ImageSamplerDescription>	ImageSamplers;
		STL::UMap<uint32_t, UniformBufferDescription>	UniformBuffers;

		STL::UMap<STL::String, VkWriteDescriptorSet>	WriteDescriptorSets;
		STL::Vector<VkDescriptorPoolSize>				DescriptorPoolSizes;
	};

	struct ShaderResource
	{
		uint32_t Set;
		uint32_t Binding;
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
		void ReflectResourceDefinitions();
		void AllocateDescriptorSets(const VkDescriptorPool descriptorPool);

		[[nodiscard]] bool
			HasDescriptorSet(const uint32_t set);
		
		[[nodiscard]] VkDescriptorSet GetDescriptorSetHandle(const uint32_t set);
		[[nodiscard]] STL::UMap<uint32_t, VkDescriptorSet>& GetDescriptorSetHandles();

		[[nodiscard]] VkDescriptorSet 
			AllocateDescriptorSet(const uint32_t set, const VkDescriptorPool descriptorPool);

		[[nodiscard]] const STL::UMap<uint32_t, ShaderDescriptorSet>&
			GetDescriptorSets() const;

		[[nodiscard]] const STL::Vector<VkDescriptorSetLayout>& 
			GetDescriptorSetLayouts() const;
		
		[[nodiscard]] const STL::Vector<VkPipelineShaderStageCreateInfo>&
			GetPipelineStages() const;

		const ShaderResource& FindShaderResource(const STL::String& name) const;
		
		const STL::Unique<VulkanAllocator>& GetAllocator();
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;
		/* Shader source code */
		STL::UMap<EShaderType, STL::String> m_ShaderSources;
		/* Shader spirv byte code*/
		STL::UMap<EShaderType, STL::Vector<uint32_t>> m_ShaderBinaries;
		/* Pipeline stages of the shader*/
		STL::Vector<VkPipelineShaderStageCreateInfo> m_PipelineStages;
		/* Shader descriptor sets: set -> descriptor info */
		STL::UMap<uint32_t, ShaderDescriptorSet> m_ShaderDescriptorSets;
		/* Descriptor set layouts used for pipeline building */
		STL::Vector<VkDescriptorSetLayout> m_ShaderDescriptorSetLayouts;
		/* Shader resource definitions (set, binding) */
		STL::UMap<STL::String, ShaderResource> m_ShaderResourceDefinitions;
		/* Descriptor set handles */
		STL::UMap<uint32_t, VkDescriptorSet> m_DescriptorSetHandles;
	};
}