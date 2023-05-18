#pragma once
#include "Cinnamon/include/Renderer/VulkanTypes.hpp"

namespace Cinnamon {
	class Shader;
	class Texture2D;

	class Material final
	{
	private:
		struct PendingDescriptorImageWrite
		{
			uint32_t Set;
			uint32_t Binding;
			Texture2D* Texture;
		};

		struct PendingDescriptorImageArrayWrite
		{
			uint32_t Set;
			uint32_t Binding;
			std::vector<Texture2D*> Textures;
		};

		struct lol
		{
			uint32_t Set;
			uint32_t Binding;
			VkDescriptorImageInfo info;
		};

		NON_COPYABLE(Material)
	public:
		explicit Material(const STL::Unique<Shader>& shader) noexcept;
		~Material() noexcept;

		void Invalidate();
		void SetTexture(const STL::String& name, const VkDescriptorImageInfo& descriptor);
		void SetTexture(const STL::String& name, Texture2D* const texture);
		void SetTextures(const STL::String& name, Texture2D* const textures, const size_t textureCount);
	private:
		const STL::Unique<Shader>& m_Shader;

		STL::Vector<lol> m_TextureWriteslol;
		STL::Vector<PendingDescriptorImageWrite> m_TextureWrites;
		STL::Vector<PendingDescriptorImageArrayWrite> m_TextureArrayWrites;
	};
}