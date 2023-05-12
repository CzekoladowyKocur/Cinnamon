#include "Cinnamon/include/Renderer/DescriptorPool.hpp"
#include "Cinnamon/include/Renderer/Device.hpp"
#include "Cinnamon/include/Renderer/GraphicsContext.hpp"

namespace Cinnamon {
	DescriptorPool::DescriptorPool(
		const STL::Unique<Device>& device,
		const size_t poolCount) noexcept
		:
		m_Device(device),
		m_Pools(poolCount, VK_NULL_HANDLE)
	{
		constexpr uint32_t descriptorPoolSize = 1000;
		const VkDescriptorPoolSize descriptorPoolSizes[11] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, descriptorPoolSize },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, descriptorPoolSize }
		};

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.maxSets = CIN_CARRAY_SIZE(descriptorPoolSizes) * descriptorPoolSize;
		descriptorPoolCreateInfo.poolSizeCount = CIN_CARRAY_SIZE(descriptorPoolSizes);
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
		descriptorPoolCreateInfo.flags = 0;
		descriptorPoolCreateInfo.pNext = nullptr;

		for (size_t i{ 0U }; i < poolCount; ++i)
		{
			VK_CHECK(vkCreateDescriptorPool(
				m_Device->GetLogicalDevice(),
				&descriptorPoolCreateInfo,
				GraphicsContext::GetAllocator(),
				&m_Pools[i]));
		}
	}

	DescriptorPool::~DescriptorPool() noexcept
	{
		for (size_t i{ 0U }; i < m_Pools.size(); ++i)
		{
			vkDestroyDescriptorPool(
				m_Device->GetLogicalDevice(),
				m_Pools[i],
				GraphicsContext::GetAllocator());
		}
	}

	void DescriptorPool::ResetPool(const uint32_t poolIndex)
	{
		VK_CHECK(vkResetDescriptorPool(
			m_Device->GetLogicalDevice(),
			m_Pools[poolIndex],
			0U));
	}

	VkDescriptorSet DescriptorPool::AllocateDescriptorSet(
		VkDescriptorSetAllocateInfo& allocationInfo,
		const uint32_t poolIndex) const
	{
		CIN_ASSERT(!allocationInfo.descriptorPool)
		allocationInfo.descriptorPool = m_Pools[poolIndex];

		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		VK_CHECK(vkAllocateDescriptorSets(
			m_Device->GetLogicalDevice(),
			&allocationInfo,
			&descriptorSet));

		return descriptorSet;
	}

	VkDescriptorPool DescriptorPool::GetPool(const uint32_t poolIndex) const
	{
		return m_Pools[poolIndex];
	}
}