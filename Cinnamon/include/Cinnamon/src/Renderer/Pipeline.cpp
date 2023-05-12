#include "Cinnamon/include/Renderer/Pipeline.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/Framebuffer.hpp"
#include "Cinnamon/include/Renderer/Shader.hpp"
#include "Cinnamon/include/Renderer/VertexBuffer.hpp"

namespace Cinnamon {
	Pipeline::Pipeline(
		const STL::Unique<Device>& device,
		const STL::Unique<Framebuffer>& framebuffer,
		const STL::Unique<Shader>& shader,
		const VertexBufferLayout& vertexBufferLayout,
		const EPrimitiveTopology topology) noexcept
		:
		m_Device(device),
		m_Framebuffer(framebuffer),
		m_Shader(shader),
		m_VertexBufferLayout(vertexBufferLayout),
		m_Topology(topology),
		m_Handle(VK_NULL_HANDLE),
		m_PipelineCache(VK_NULL_HANDLE),
		m_Layout(VK_NULL_HANDLE)
	{
		Invalidate();
	}

	Pipeline::~Pipeline() noexcept
	{
		[[likely]]
		if (m_Layout)
		{
			vkDestroyPipelineLayout(
				m_Device->GetLogicalDevice(), 
				m_Layout, 
				GraphicsContext::GetAllocator());
		}

		[[likely]]
		if (m_Handle) 
		{
			vkDestroyPipeline(
				m_Device->GetLogicalDevice(),
				m_Handle,
				GraphicsContext::GetAllocator());
		}
	}

	void Pipeline::Invalidate()
	{
		const auto& descriptorSetLayouts = m_Shader->GetDescriptorSetLayouts();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.flags = 0;
		pipelineLayoutCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreatePipelineLayout(
			m_Device->GetLogicalDevice(),
			&pipelineLayoutCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Layout));

		STL::Vector<VkVertexInputAttributeDescription> vertexInputAttributes(m_VertexBufferLayout.ElementCount);
		
		for (size_t i{ 0U }; i < m_VertexBufferLayout.Elements.size(); ++i)
		{
			const auto& element{ m_VertexBufferLayout.Elements[i] };

			vertexInputAttributes[i].binding = 0;
			vertexInputAttributes[i].location = static_cast<uint32_t>(i);
			vertexInputAttributes[i].offset = element.Offset;
			vertexInputAttributes[i].format = static_cast<VkFormat>(element.ElementType);
		}

		VkVertexInputBindingDescription vertexInputBindingDescription;
		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindingDescription.stride = m_VertexBufferLayout.Stride;

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo;
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.pNext = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = static_cast<VkPrimitiveTopology>(m_Topology);
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
		inputAssemblyCreateInfo.flags = 0;
		inputAssemblyCreateInfo.pNext = nullptr;

		VkPipelineTessellationStateCreateInfo tesselationStateCreateInfo;
		tesselationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tesselationStateCreateInfo.patchControlPoints = 0;
		tesselationStateCreateInfo.flags = 0;
		tesselationStateCreateInfo.pNext = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
		rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.polygonMode = false ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT; //VK_CULL_MODE_BACK_BIT; /* TODO: adjustable */
		rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
		rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
		rasterizationStateCreateInfo.lineWidth = 1.0f;
		rasterizationStateCreateInfo.flags = 0;
		rasterizationStateCreateInfo.pNext = nullptr;

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
		auto& blendAttachmentState{ blendAttachmentStates.emplace_back(VkPipelineColorBlendAttachmentState{}) };


		blendAttachmentState.colorWriteMask = 0xf;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;//VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; //VK_BLEND_FACTOR_ZERO;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
		depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
		depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
		depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateCreateInfo.minDepthBounds = 0.0f;
		depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
		depthStencilStateCreateInfo.stencilTestEnable = VK_TRUE;
		depthStencilStateCreateInfo.front = {};
		depthStencilStateCreateInfo.back = {};
		depthStencilStateCreateInfo.flags = 0;
		depthStencilStateCreateInfo.pNext = nullptr;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateInfo.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
		colorBlendStateInfo.pAttachments = blendAttachmentStates.data();
		colorBlendStateInfo.flags = 0;
		colorBlendStateInfo.pNext = nullptr;

		/* Will be overriden by dynamic state */
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;
		viewportState.flags = 0;
		viewportState.pNext = nullptr;

		STL::Array<VkDynamicState, 2U> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();
		dynamicStateInfo.flags = 0;
		dynamicStateInfo.pNext = nullptr;

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
		multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.rasterizationSamples = static_cast<VkSampleCountFlagBits>(VK_SAMPLE_COUNT_1_BIT);
		multisampleStateCreateInfo.pSampleMask = nullptr;

		const auto& pipelineStages = m_Shader->GetPipelineStages();

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
		graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.layout = m_Layout;
		graphicsPipelineCreateInfo.renderPass = m_Framebuffer->GetRenderPass();
		graphicsPipelineCreateInfo.subpass = 0;
		graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineStages.size());
		graphicsPipelineCreateInfo.pStages = pipelineStages.data();
		graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateInfo;
		graphicsPipelineCreateInfo.pTessellationState = &tesselationStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateInfo; //colorBlending; 
		graphicsPipelineCreateInfo.pViewportState = &viewportState;
		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
		graphicsPipelineCreateInfo.basePipelineHandle = 0;
		graphicsPipelineCreateInfo.basePipelineIndex = 0;
		graphicsPipelineCreateInfo.flags = 0;
		graphicsPipelineCreateInfo.pNext = nullptr;

		VK_CHECK(vkCreateGraphicsPipelines(
			m_Device->GetLogicalDevice(),
			m_PipelineCache,
			1,
			&graphicsPipelineCreateInfo,
			GraphicsContext::GetAllocator(),
			&m_Handle));
	}

	VkPipeline Pipeline::GetHandle() const
	{
		return m_Handle;
	}

	VkPipelineLayout Pipeline::GetLayout() const
	{
		return m_Layout;
	}
}