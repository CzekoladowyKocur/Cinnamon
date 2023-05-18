#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "shaderc/env.h"
#include <cmath>

#ifdef CIN_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 26439)
#include "shaderc/shaderc.h"
/* Use the self-compiled spirv-cross instead of the SDK one. */
#define SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#include "ThirdParty/spirv_cross/spirv_cross.hpp"
#pragma warning(pop)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#define SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
#include "ThirdParty/spirv_cross/spirv_cross.hpp"
#include "shaderc/shaderc.h"
#pragma GCC diagnostic pop
#endif

namespace Cinnamon {
	InternalScope STL::UMap<EShaderType, STL::String> PreprocessShaderSource(std::ifstream& file) noexcept;
	InternalScope STL::UMap<EShaderType, STL::Vector<uint32_t>> ProcessShaderSources(const STL::UMap<EShaderType, STL::String>& shaderSources, const STL::Filepath& shaderFilepath, const bool compile) noexcept;
	InternalScope EShaderType ShaderTypeStringToType(const STL::StringView shaderType) noexcept;
	InternalScope VkShaderStageFlagBits CinnamonShaderStageToVulkanShaderStage(const EShaderType shaderType) noexcept;
	InternalScope shaderc_env_version CinnamonVulkanVersionToShaderCEnvironment() noexcept;
	InternalScope shaderc_shader_kind CinnamonShaderTypeToShaderCShaderType(const EShaderType shaderType) noexcept;

	Shader::Shader(
		const STL::Unique<VulkanAllocator>& allocator,
		const STL::Filepath& filepath,
		const bool forceCompile) noexcept
		:
		m_Allocator(allocator),
		m_ShaderSources(),
		m_ShaderBinaries(),
		m_PipelineStages(),
		m_ShaderDescriptorSets(),
		m_ShaderDescriptorSetLayouts()
	{
		std::ifstream shaderFile(filepath, std::ios::in | std::ios::binary);
		CIN_ASSERT(shaderFile.is_open());

		CIN_INFO("Preprocessing shader {}", filepath.string());
		m_ShaderSources = PreprocessShaderSource(shaderFile);
		CIN_INFO("Compiling shader {}", filepath.string());
		m_ShaderBinaries = ProcessShaderSources(m_ShaderSources, filepath, forceCompile);

		for(const auto& [shaderType, shaderBinary] : m_ShaderBinaries)
		{ 
			const VkShaderModuleCreateInfo shaderModuleCreateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.codeSize{ shaderBinary.size() * sizeof(uint32_t) },
				.pCode{ shaderBinary.data() }
			};

			VkShaderModule shaderModule{ VK_NULL_HANDLE };
			VK_CHECK(vkCreateShaderModule(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				&shaderModuleCreateInfo,	
				GraphicsContext::GetAllocator(),
				&shaderModule));

			VkPipelineShaderStageCreateInfo pipelineStage
			{
				.sType{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.stage{ CinnamonShaderStageToVulkanShaderStage(shaderType) },
				.module{ shaderModule },
				.pName{ "main" },
				.pSpecializationInfo{ nullptr }
			};

			m_PipelineStages.emplace_back(std::move(pipelineStage));
		}

		Reflect();
		ReflectResourceDefinitions();
	}
	
	Shader::~Shader() noexcept
	{
		for (const VkPipelineShaderStageCreateInfo& pipelineStage : m_PipelineStages)
		{
			vkDestroyShaderModule(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				pipelineStage.module,
				GraphicsContext::GetAllocator());
		}

		m_PipelineStages.clear();
		for (const VkDescriptorSetLayout descriptorSetLayotut : m_ShaderDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				descriptorSetLayotut,
				GraphicsContext::GetAllocator());
		}

		m_ShaderDescriptorSetLayouts.clear();
	}

	void Shader::Reflect()
	{
		for (const auto& [stage, binary] : m_ShaderBinaries)
		{
			const spirv_cross::Compiler compiler(binary);
			const spirv_cross::ShaderResources resources{ compiler.get_shader_resources() };

			for (const auto& uniformBuffer : resources.uniform_buffers)
			{
				const STL::String& uniformBufferName{ uniformBuffer.name };
				const uint32_t binding{ compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding) };
				const uint32_t descriptorSet{ compiler.get_decoration(uniformBuffer.id, spv::DecorationDescriptorSet) };

				CIN_INFO("Found uniform buffer at binding {} with name {} in descriptor set {}", binding, uniformBufferName, descriptorSet);

				m_ShaderDescriptorSets[descriptorSet].UniformBuffers[binding] = UniformBufferDescription
				{
					.Name{ uniformBufferName },
					.BindingPoint{ binding },
					.DescriptorSet{ descriptorSet }
				};
			}

			for (const auto& sampledImage : resources.sampled_images)
			{
				const STL::String& sampledImageName{ sampledImage.name };
				const uint32_t binding{ compiler.get_decoration(sampledImage.id, spv::DecorationBinding) };
				const uint32_t descriptorSet{ compiler.get_decoration(sampledImage.id, spv::DecorationDescriptorSet) };
			
				CIN_INFO("Found sampled image at binding {} with name {} in descriptor set {}", binding, sampledImageName, descriptorSet);

				m_ShaderDescriptorSets[descriptorSet].ImageSamplers[binding] = ImageSamplerDescription
				{
					.Name{ sampledImageName },
					.BindingPoint{ binding },
					.DescriptorSet{ descriptorSet },
					.ArraySize{ 1U }
				};
			}			
		}

		for (size_t shaderDescriptorSetIndex{ 0U }; shaderDescriptorSetIndex < m_ShaderDescriptorSets.size(); ++shaderDescriptorSetIndex)
		{
			auto& shaderDescriptorSet{ m_ShaderDescriptorSets[(uint32_t)shaderDescriptorSetIndex] };

			VkDescriptorPoolSize descriptorPoolSize;
			if (const uint32_t resourceCount = static_cast<uint32_t>(shaderDescriptorSet.ImageSamplers.size()))
			{
				descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorPoolSize.descriptorCount = resourceCount;

				shaderDescriptorSet.DescriptorPoolSizes.emplace_back(std::move(descriptorPoolSize));
			}

			STL::Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
			for (const auto& [binding, uniformBuffer] : shaderDescriptorSet.UniformBuffers)
			{
				const VkDescriptorSetLayoutBinding descriptorSetLayoutBinding
				{
					.binding{ binding },
					.descriptorType{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
					.descriptorCount{ 1U },
					.stageFlags{ uniformBuffer.ShaderStage },
					.pImmutableSamplers{ nullptr }
				};

				VkWriteDescriptorSet writeDescriptorSet
				{
					.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
					.pNext{ nullptr },
					.dstSet{ VK_NULL_HANDLE },
					.dstBinding{ binding },
					.dstArrayElement{ 0U },
					.descriptorCount{ 0U },
					.descriptorType{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
					.pImageInfo{ nullptr },
					.pBufferInfo{ nullptr },
					.pTexelBufferView{ nullptr }
				};

				descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);
				shaderDescriptorSet.WriteDescriptorSets[uniformBuffer.Name] = std::move(writeDescriptorSet);
			}

			for (const auto& [binding, imageSampler] : shaderDescriptorSet.ImageSamplers)
			{
				const VkDescriptorSetLayoutBinding descriptorSetLayoutBinding
				{
					.binding{ binding },
					.descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
					.descriptorCount{ imageSampler.ArraySize },
					.stageFlags{ imageSampler.ShaderStage },
					.pImmutableSamplers{ nullptr }
				};

				VkWriteDescriptorSet writeDescriptorSet
				{
					.sType{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET },
					.pNext{ nullptr },
					.dstSet{ VK_NULL_HANDLE },
					.dstBinding{ binding },
					.dstArrayElement{ 0U },
					.descriptorCount{ imageSampler.ArraySize },
					.descriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
					.pImageInfo{ nullptr },
					.pBufferInfo{ nullptr },
					.pTexelBufferView{ nullptr }
				};

				descriptorSetLayoutBindings.emplace_back(descriptorSetLayoutBinding);
				shaderDescriptorSet.WriteDescriptorSets[imageSampler.Name] = std::move(writeDescriptorSet);
			}

			/* Make sure our descriptor set layout vector can hold all of the layouts */
			if (shaderDescriptorSetIndex >= m_ShaderDescriptorSetLayouts.size())
				m_ShaderDescriptorSetLayouts.resize(shaderDescriptorSetIndex + 1);

			const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo
			{
				.sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO },
				.pNext{ nullptr },
				.flags{ 0U },
				.bindingCount{ static_cast<uint32_t>(descriptorSetLayoutBindings.size()) },
				.pBindings{ descriptorSetLayoutBindings.empty() ? nullptr : descriptorSetLayoutBindings.data() }
			};

			VK_CHECK(vkCreateDescriptorSetLayout(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				&descriptorSetLayoutInfo,
				GraphicsContext::GetAllocator(),
				&m_ShaderDescriptorSetLayouts[shaderDescriptorSetIndex]));

			++shaderDescriptorSetIndex;
		}
	}

	void Shader::ReflectResourceDefinitions()
	{
		CIN_INFO("Reflecting shader resources");
		for (const auto& [descriptorSetIndex, descriptorSet] : m_ShaderDescriptorSets)
		{
			CIN_INFO("Reflecting descriptor set {}", descriptorSetIndex);
			
			CIN_INFO("Reflecting uniform buffers");
			for (const auto& [uniformBufferBinding, uniformBuffer] : descriptorSet.UniformBuffers)
			{
				CIN_INFO("Reflected uniform buffer {} in descriptor set {} at binding {}", uniformBuffer.Name, uniformBuffer.DescriptorSet, uniformBuffer.BindingPoint);
				m_ShaderResourceDefinitions[uniformBuffer.Name] =
				{
					ShaderResource
					{
						.Set{ uniformBuffer.DescriptorSet },
						.Binding{ uniformBufferBinding }
					}
				};
			}
			
			for (const auto& [samplerBinding, sampler] : descriptorSet.ImageSamplers)
			{
				CIN_INFO("Reflected image sampler {} in descriptor set {} at binding {}", sampler.Name, sampler.DescriptorSet, sampler.BindingPoint);
				m_ShaderResourceDefinitions[sampler.Name] =
				{
					ShaderResource
					{
						.Set{ sampler.DescriptorSet },
						.Binding{ samplerBinding }
					}
				};
			}
		}
	}

	void Shader::AllocateDescriptorSets(const VkDescriptorPool descriptorPool)
	{
		for (const auto& [descriptorSetIndex, descriptorSet] : m_ShaderDescriptorSets)
		{
			const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
			{
				.sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO },
				.pNext{ nullptr },
				.descriptorPool{ descriptorPool },
				.descriptorSetCount{ 1U },
				.pSetLayouts{ &m_ShaderDescriptorSetLayouts[descriptorSetIndex] },
			};

			VkDescriptorSet descriptorSetHandle;
			VK_CHECK(vkAllocateDescriptorSets(
				m_Allocator->GetDevice()->GetLogicalDevice(),
				&descriptorSetAllocateInfo,
				&descriptorSetHandle));

			m_DescriptorSetHandles[descriptorSetIndex] = descriptorSetHandle;
		}
	}

	STL::UMap<uint32_t, VkDescriptorSet>& Shader::GetDescriptorSetHandles()
	{
		return m_DescriptorSetHandles;
	}

	bool Shader::HasDescriptorSet(const uint32_t set)
	{
		return m_ShaderDescriptorSets.contains(set);
	}
	
	VkDescriptorSet Shader::GetDescriptorSetHandle(const uint32_t set)
	{
		CIN_ASSERT(not m_DescriptorSetHandles.empty());
		return m_DescriptorSetHandles[set];
	}

	const STL::UMap<uint32_t, ShaderDescriptorSet>& Shader::GetDescriptorSets() const
	{
		return m_ShaderDescriptorSets;
	}

	const STL::Vector<VkDescriptorSetLayout>& Shader::GetDescriptorSetLayouts() const
	{
		return m_ShaderDescriptorSetLayouts;
	}

	VkDescriptorSet Shader::AllocateDescriptorSet(const uint32_t set, const VkDescriptorPool descriptorPool)
	{
		const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
		{
			.sType{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO },
			.pNext{ nullptr },
			.descriptorPool{ descriptorPool },
			.descriptorSetCount{ 1U },
			.pSetLayouts{ &m_ShaderDescriptorSetLayouts[set] },
		};

		VkDescriptorSet descriptorSet;
		VK_CHECK(vkAllocateDescriptorSets(
			m_Allocator->GetDevice()->GetLogicalDevice(),
			&descriptorSetAllocateInfo,
			&descriptorSet));

		return descriptorSet;
	}

	const STL::Vector<VkPipelineShaderStageCreateInfo>& Shader::GetPipelineStages() const
	{
		return m_PipelineStages;
	}

	const STL::Unique<VulkanAllocator>& Shader::GetAllocator()
	{
		return m_Allocator;
	}

	const ShaderResource& Shader::FindShaderResource(const STL::String& name) const
	{
		CIN_ASSERT(m_ShaderResourceDefinitions.contains(name));
		return m_ShaderResourceDefinitions.at(name);
	}

	InternalScope STL::UMap<EShaderType, STL::String> PreprocessShaderSource(std::ifstream& file) noexcept
	{
		std::stringstream fileContents;
		fileContents << file.rdbuf();
		file.close();
		
		STL::String currentLine;
		STL::String currentShaderSource;
		EShaderType currentShaderType{ EShaderType::None };
		STL::UMap<EShaderType, STL::String> shaderSources;

		while (std::getline(fileContents, currentLine))
		{
			if (currentLine.find('#') != std::string::npos)
			{			
				const STL::String shaderTypeTokenenized{ currentLine.substr(currentLine.find('#')) };
				/* Check if it's an empty token */
				if (shaderTypeTokenenized.size() > 1U)
				{
					/* Skip token symbol */
					STL::String shaderType{ shaderTypeTokenenized.substr(1U) };
					
					/* Remove '\r' and '\n' characters */
					size_t formatter{ shaderType.find('\r') };
					while (formatter != STL::String::npos)
					{
						shaderType.erase(formatter);
						formatter = { shaderType.find('\r') };
					}

					formatter = { shaderType.find('\n') };
					while (formatter != STL::String::npos)
					{
						shaderType.erase(formatter);
						formatter = { shaderType.find('\n') };
					}

					formatter = { shaderType.find(' ') };
					while (formatter != STL::String::npos)
					{
						shaderType.erase(formatter);
						formatter = { shaderType.find(' ') };
					}

					const EShaderType shader{ ShaderTypeStringToType(shaderType) };
					if (shader != EShaderType::None)
					{
						if (currentShaderType != EShaderType::None)
						{
							shaderSources[currentShaderType] = std::move(currentShaderSource);
							currentShaderSource.clear();
						}

						currentShaderType = shader;
						/* Continue reading after the specified shader type */
						continue;
					}
				}

				currentShaderSource += currentLine + '\n';
				continue;
			}

			if(currentShaderType != EShaderType::None)
				currentShaderSource += currentLine + '\n';
		}

		if (currentShaderType != EShaderType::None)
		{
			shaderSources[currentShaderType] = std::move(currentShaderSource);

			currentShaderType = EShaderType::None;
			currentShaderSource.clear();
		}

		return shaderSources;
	}

	InternalScope STL::UMap<EShaderType, STL::Vector<uint32_t>> ProcessShaderSources(
		const STL::UMap<EShaderType, STL::String>& shaderSources, 
		const STL::Filepath& shaderFilepath,
		const bool compile) noexcept
	{
		STL::UMap<EShaderType, STL::Vector<uint32_t>> spirvBinaries;

		bool compiledShadersExist{ true };
		const STL::String shaderFilepathStem{ (shaderFilepath.parent_path() / shaderFilepath.stem()).string() };
		for (const auto& [shaderStage, _] : shaderSources)
		{
			if (!std::filesystem::exists(shaderFilepathStem + "." + ShaderTypeToString(shaderStage)))
			{
				compiledShadersExist = false;
				break;
			}
		}

		for (const auto& [shaderStage, shaderSource] : shaderSources)
		{		
			if (compile || !compiledShadersExist)
			{
				shaderc_compiler_t compiler{ shaderc_compiler_initialize() };
				shaderc_compile_options_t compileOptions{ shaderc_compile_options_initialize() };
				shaderc_compile_options_set_source_language(compileOptions, shaderc_source_language_glsl);
				shaderc_compile_options_set_target_env(compileOptions, shaderc_target_env_vulkan, CinnamonVulkanVersionToShaderCEnvironment());

				const STL::String shaderFilepathString{ shaderFilepath.string() };
				
				const shaderc_compilation_result_t compilationResult
				{
					
					shaderc_compile_into_spv
					(
						compiler,
						shaderSource.data(),
						shaderSource.size(),
						CinnamonShaderTypeToShaderCShaderType(shaderStage),
						shaderFilepathString.data(),
						"main",
						compileOptions
					)
				};

				if (shaderc_result_get_compilation_status(compilationResult) != shaderc_compilation_status_success)
					CIN_ERROR("{}", shaderc_result_get_error_message(compilationResult));
				
				CIN_ASSERT(shaderc_result_get_compilation_status(compilationResult) == shaderc_compilation_status_success);
				
				const void* binaryData = shaderc_result_get_bytes(compilationResult);
				const size_t binarySize = shaderc_result_get_length(compilationResult);

				const uint32_t* binaryDataAsUint32 = reinterpret_cast<const uint32_t*>(binaryData);
				std::vector<uint32_t> bytecode(binaryDataAsUint32, binaryDataAsUint32 + (binarySize / sizeof(uint32_t)));

				spirvBinaries[shaderStage] = std::move(bytecode);

				shaderc_compiler_release(compiler);
				shaderc_compile_options_release(compileOptions);
				shaderc_result_release(compilationResult);

				const STL::String outputPath{ shaderFilepathStem + "." + ShaderTypeToString(shaderStage) };
				FILE* file;
				fopen_s(&file, outputPath.c_str(), "wb+");
				CIN_ASSERT(file);
				fwrite(spirvBinaries[shaderStage].data(), sizeof(uint32_t), spirvBinaries[shaderStage].size(), file);
				fclose(file);
			}
			else
			{
				const STL::String outputPath{ shaderFilepathStem + "." + ShaderTypeToString(shaderStage) };
				FILE* file;
				fopen_s(&file, outputPath.c_str(), "rb");
				
				CIN_ASSERT(file);
				fseek(file, 0, SEEK_END);
				const uint64_t size = ftell(file);
				spirvBinaries[shaderStage] = std::vector<uint32_t>(size / sizeof(uint32_t));
				fseek(file, 0, SEEK_SET);
				fread(spirvBinaries[shaderStage].data(), sizeof(uint32_t), spirvBinaries[shaderStage].size(), file);
				fclose(file);
			}
		}

		return spirvBinaries;
	}

	InternalScope EShaderType ShaderTypeStringToType(const STL::StringView shaderType) noexcept
	{
		STL::String processed;
		processed.resize(shaderType.size());

		for (size_t i{ 0U }; i < shaderType.size(); ++i)
			processed[i] = static_cast<char>(std::tolower(static_cast<int>(shaderType[i])));

		if (processed == "vertex")
			return EShaderType::Vertex;
		else if (processed == "fragment")
			return EShaderType::Fragment;
		else if (processed == "compute")
			return EShaderType::Compute;

		return EShaderType::None;
	}

	InternalScope VkShaderStageFlagBits CinnamonShaderStageToVulkanShaderStage(const EShaderType shaderType) noexcept
	{
		switch (shaderType)
		{
			case EShaderType::Vertex:	return VK_SHADER_STAGE_VERTEX_BIT;
			case EShaderType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
			case EShaderType::Compute:	return VK_SHADER_STAGE_COMPUTE_BIT;

			[[unlikely]]
			default: CIN_ASSERT(false); return VK_SHADER_STAGE_VERTEX_BIT;
		}
	}

	InternalScope shaderc_env_version CinnamonVulkanVersionToShaderCEnvironment() noexcept
	{
		CIN_ASSERT(GraphicsContext::GetAPIVersion() == VK_API_VERSION_1_3);
		return shaderc_env_version_vulkan_1_3;
	}

	InternalScope shaderc_shader_kind CinnamonShaderTypeToShaderCShaderType(const EShaderType shaderType) noexcept
	{
		switch (shaderType)
		{
			case EShaderType::Vertex:	return shaderc_shader_kind::shaderc_vertex_shader;
			case EShaderType::Fragment:	return shaderc_shader_kind::shaderc_fragment_shader;
			case EShaderType::Compute:	return shaderc_shader_kind::shaderc_compute_shader;

			[[unlikely]]
			default: CIN_ASSERT(false); return shaderc_shader_kind::shaderc_vertex_shader;
		}
	}
}