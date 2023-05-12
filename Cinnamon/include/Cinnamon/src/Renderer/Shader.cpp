#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "shaderc/env.h"
#include <cmath>


#ifdef CIN_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 26439)
#include "shaderc/shaderc.hpp"
#include "spirv_cross/spirv_glsl.hpp"
#pragma warning(pop)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#include "shaderc/shaderc.h"
//#include "glslang/Public/ShaderLang.h"
//#include "glslang/MachineIndependent/localintermediate.h"
//#include "glslang/SPIRV/GlslangToSpv.h"
//#include "glslang/Include/ResourceLimits.h"
//#include "spirv_cross/spirv.h"
#pragma GCC diagnostic pop
#endif

namespace Cinnamon {
	InternalScope STL::UMap<EShaderType, STL::String> PreprocessShaderSource(std::ifstream& file) noexcept;
	InternalScope STL::UMap<EShaderType, STL::Vector<uint32_t>> ProcessShaderSources(const STL::UMap<EShaderType, STL::String>& shaderSources, const STL::Filepath& shaderFilepath, const bool compile) noexcept;
	InternalScope EShaderType ShaderTypeStringToType(const STL::StringView shaderType) noexcept;
	InternalScope VkShaderStageFlagBits CinnamonShaderStageToVulkanShaderStage(const EShaderType shaderType) noexcept;
	InternalScope shaderc_env_version CinnamonVulkanVersionToShaderCEnvironment() noexcept;
	InternalScope shaderc_shader_kind CinnamonShaderTypeToShaderCShaderType(const EShaderType shaderType) noexcept;
#if 0
	InternalScope EShLanguage CinnamonShaderTypeToShaderShaderKind(const EShaderType shaderType) noexcept
	{
		switch (shaderType)
		{
			case EShaderType::Vertex:	return EShLangVertex;
			case EShaderType::Fragment:	return EShLangFragment;
			case EShaderType::Compute:	return EShLangCompute;

			[[unlikely]]
			default: CIN_ASSERT(false); return EShLangVertex;
		}
	}
#endif

	Shader::Shader(
		const STL::Unique<VulkanAllocator>& allocator,
		const STL::Filepath& filepath,
		const bool forceCompile) noexcept
		:
		m_Allocator(allocator),
		m_PipelineStages(),
		m_DescriptorSetLayouts(),
		m_ShaderSources(),
		m_ShaderBinaries()
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

			const VkPipelineShaderStageCreateInfo pipelineStage
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
	}

	void Shader::Reflect()
	{
		//for (const auto& [stage, binary] : m_ShaderBinaries)
		{
			//const spirv_cross::Compiler compiler(binary);
			//const spirv_cross::ShaderResources resources{ compiler.get_shader_resources() };

			/* Iterate over all of the resources */
			//for (const auto& ub : resources.uniform_buffers)
			//{
			//}
		}
	}

	const STL::Vector<VkDescriptorSetLayout>& Shader::GetDescriptorSetLayouts() const
	{
		return m_DescriptorSetLayouts;
	}

	const STL::Vector<VkPipelineShaderStageCreateInfo>& Shader::GetPipelineStages() const
	{
		return m_PipelineStages;
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

				CIN_ASSERT(shaderc_result_get_compilation_status(compilationResult) == shaderc_compilation_status_success);


				size_t size = shaderc_result_get_length(compilationResult);
    			const char* binary = shaderc_result_get_bytes(compilationResult);

				spirvBinaries[shaderStage] = STL::Vector<uint32_t>{ binary, binary + size };

				shaderc_compiler_release(compiler);
				shaderc_compile_options_release(compileOptions);
				shaderc_result_release(compilationResult);
				#if 0
				shaderc::Compiler compiler;
				shaderc::CompileOptions compileOptions;

				compileOptions.SetTargetSpirv(shaderc_spirv_version_1_6);
				compileOptions.SetWarningsAsErrors();
				compileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, CinnamonVulkanVersionToShaderCEnvironment());

				const shaderc_shader_kind shaderKind{ CinnamonShaderTypeToShaderCShaderType(shaderStage) };
				const STL::String shaderFilepathString{ shaderFilepath.string() };

				const shaderc::SpvCompilationResult compilationResult
				{
					compiler.CompileGlslToSpv
					(
						shaderSource,
						shaderKind,
						shaderFilepathString.data(),
						compileOptions
					)
				};

				if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success)
					CIN_ERROR("Shader compilation failed: {}", compilationResult.GetErrorMessage());
				CIN_VERIFY(compilationResult.GetCompilationStatus() == shaderc_compilation_status_success);

				spirvBinaries[shaderStage] = STL::Vector<uint32_t>(compilationResult.begin(), compilationResult.end());
#endif
#if 0
				auto shaderKind{ CinnamonShaderTypeToShaderShaderKind(shaderStage) };
				glslang::TProgram program;
				glslang::TShader shader(shaderKind);
				const char* shaderSourcePtr = shaderSource.c_str();

				shader.setStrings(&shaderSourcePtr, 1);
				shader.setEnvInput(glslang::EShSourceGlsl, shaderKind, glslang::EShClientVulkan, 100);
				shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
				shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);



				if (!shader.parse(&resources, 100, false, messages))
				{
    				CIN_ERROR("Shader compilation failed: {}", messages.c_str());
				}

				program.addShader(&shader);

				if (!program.link(messages))
				{
    				CIN_ERROR("Shader linking failed: {}", messages.c_str());
				}

				spv::SpvBuildLogger logger;
				glslang::SpvOptions spvOptions;
				spvOptions.generateDebugInfo = false;
				spvOptions.disableOptimizer = true;
				spvOptions.optimizeSize = true;

				std::vector<unsigned int> spirvCode;

				glslang::GlslangToSpv(*program.getIntermediate(shaderKind), spirvCode, &logger, &spvOptions);

				spirvBinaries[shaderStage] = spirvCode;
#endif


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
#if 1
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
#endif
}