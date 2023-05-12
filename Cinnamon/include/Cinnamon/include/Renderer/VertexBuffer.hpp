#pragma once
#include "Cinnamon/include/Renderer/VulkanAllocator.hpp"

namespace Cinnamon {
	enum class EShaderDataType
	{
		None	= VK_FORMAT_UNDEFINED,
		Int1	= VK_FORMAT_R32_SINT,
		Int2	= VK_FORMAT_R32G32_SINT,
		Int3	= VK_FORMAT_R32G32B32_SINT,
		Int4	= VK_FORMAT_R32G32B32A32_SINT,
		UInt1	= VK_FORMAT_R32_UINT,
		UInt2	= VK_FORMAT_R32G32_UINT,
		UInt3	= VK_FORMAT_R32G32B32_UINT,
		UInt4	= VK_FORMAT_R32G32B32A32_UINT,
		Float1	= VK_FORMAT_R32_SFLOAT,
		Float2	= VK_FORMAT_R32G32_SFLOAT,
		Float3	= VK_FORMAT_R32G32B32_SFLOAT,
		Float4	= VK_FORMAT_R32G32B32A32_SFLOAT,
	};

	constexpr uint32_t ShaderDataTypeCount(const EShaderDataType elementType);
	constexpr uint32_t ShaderDataTypeSize(const EShaderDataType elementType);

	struct VertexBufferElement
	{
		EShaderDataType ElementType;
		uint32_t Size, ComponentCount, Offset;
		bool Normalized;

		VertexBufferElement(const EShaderDataType elementType)
			:
			ElementType(elementType),
			Size(ShaderDataTypeSize(elementType)),
			ComponentCount(ShaderDataTypeCount(elementType)),
			Offset(0),
			Normalized(false)
		{}
	};

	struct VertexBufferLayout
	{
		STL::Vector<VertexBufferElement> Elements;
		uint32_t Stride;
		uint32_t ElementCount;

		constexpr explicit VertexBufferLayout(const STL::InitializerList<VertexBufferElement>& elements) noexcept
			:
			Elements(elements),
			Stride(0),
			ElementCount(0)
		{
			for (VertexBufferElement& element : Elements)
			{
				element.Offset += Stride;
				Stride += element.Size;
				++ElementCount;
			}
		}

		STL::Vector<VertexBufferElement>::iterator begin();
		STL::Vector<VertexBufferElement>::iterator end();
		STL::Vector<VertexBufferElement>::const_iterator cbegin() const;
		STL::Vector<VertexBufferElement>::const_iterator cend() const;
	};

	class VertexBuffer final
	{
	private:
		NON_COPYABLE(VertexBuffer)
	public:
		explicit VertexBuffer(
			const STL::Unique<VulkanAllocator>& allocator, 
			const VkDeviceSize reservedSize,
			const VertexBufferLayout& layout) noexcept;

		~VertexBuffer() noexcept;
		
		[[nodiscard]] VkBuffer
			GetHandle() const;
		
		[[nodiscard]] const VertexBufferLayout& 
			GetLayout() const;

		void SetData(const void* data, const VkDeviceSize size, const VkDeviceSize offset = 0U);
	private:
		const STL::Unique<VulkanAllocator>& m_Allocator;

		VkBuffer m_Handle;
		VmaAllocation m_DeviceAllocation;
		VertexBufferLayout m_Layout;
	};

	constexpr uint32_t ShaderDataTypeCount(const EShaderDataType elementType)
	{
		switch (elementType)
		{
			case EShaderDataType::None:		return 0;
			case EShaderDataType::Float1:	return 1;
			case EShaderDataType::Float2:	return 2;
			case EShaderDataType::Float3:	return 3;
			case EShaderDataType::Float4:	return 4;

			case EShaderDataType::Int1:		return 1;
			case EShaderDataType::Int2:		return 2;
			case EShaderDataType::Int3:		return 3;
			case EShaderDataType::Int4:		return 4;

			case EShaderDataType::UInt1:	return 1;
			case EShaderDataType::UInt2:	return 2;
			case EShaderDataType::UInt3:	return 3;
			case EShaderDataType::UInt4:	return 4;

			[[unlikely]]
			default:
			{
				CIN_ASSERT(false, "Unknown shader data type");
				return 0;
			}
		}
	}

	constexpr uint32_t ShaderDataTypeSize(const EShaderDataType elementType)
	{
		switch (elementType)
		{
			case EShaderDataType::None:		return 0;
			case EShaderDataType::Float1:	return 4 * 1;
			case EShaderDataType::Float2:	return 4 * 2;
			case EShaderDataType::Float3:	return 4 * 3;
			case EShaderDataType::Float4:	return 4 * 4;

			case EShaderDataType::Int1:		return 4 * 1;
			case EShaderDataType::Int2:		return 4 * 2;
			case EShaderDataType::Int3:		return 4 * 3;
			case EShaderDataType::Int4:		return 4 * 4;

			case EShaderDataType::UInt1:	return 4 * 1;
			case EShaderDataType::UInt2:	return 4 * 2;
			case EShaderDataType::UInt3:	return 4 * 3;
			case EShaderDataType::UInt4:	return 4 * 4;

			[[unlikely]]
			default:
			{
				CIN_ASSERT(false, "Unknown shader data type");
				return 0;
			}			
		}
	}
}