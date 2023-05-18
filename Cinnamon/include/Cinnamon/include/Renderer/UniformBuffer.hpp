#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

namespace Cinnamon {
	class UniformBuffer final
	{
	private:
		NON_COPYABLE(UniformBuffer)
	public:
		explicit UniformBuffer(
			const STL::Unique<VulkanAllocator>& allocator,
			const size_t size,
			const uint32_t binding) noexcept;

		~UniformBuffer() noexcept;

		void SetData(
			const void* const data, 
			const uint64_t size, 
			const uint64_t offset = 0U);

		Byte* MapData();
		void Unmapdata();

		[[nodiscard]] uint32_t
			GetBindingSlot() const;

		[[nodiscard]] const VkDescriptorBufferInfo&
			GetDescriptorBufferInfo() const;
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;

		uint32_t m_BindingSlot;
		VkBuffer m_Handle;
		VmaAllocation m_DeviceAllocation;
		VkDescriptorBufferInfo m_DescriptorBufferInfo;
	};

}