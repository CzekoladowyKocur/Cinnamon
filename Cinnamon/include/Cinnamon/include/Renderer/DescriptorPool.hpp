#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	class Device;

	class DescriptorPool final
	{
	private:
		NON_COPYABLE(DescriptorPool)
	public:
		explicit DescriptorPool(
			const STL::Unique<Device>& device, 
			const size_t poolCount) noexcept;

		~DescriptorPool() noexcept;

		void ResetPool(const uint32_t poolIndex);

		[[nodiscard]] VkDescriptorSet AllocateDescriptorSet(
				VkDescriptorSetAllocateInfo& allocationInfo, 
				const uint32_t poolIndex) const;
	private:
		const STL::Unique<Device>& m_Device;
		
		STL::Vector<VkDescriptorPool> m_Pools;
	};
}