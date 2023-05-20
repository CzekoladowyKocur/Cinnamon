#include "Cinnamon/include/Scene/SceneSerializer.hpp"
#include "Cinnamon/include/Scene/Components.hpp"
#include "Cinnamon/include/Asset/AssetManager.hpp"
#include "Cinnamon/include/Scene/Scene.hpp"
#include "Cinnamon/include/ECS/Registry.hpp"
#include "Cinnamon/include/Renderer/Texture2D.hpp"

#ifdef CIN_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 5054)
#pragma warning(disable : 4251)
#pragma warning(disable : 26495)
#pragma warning(disable : 4275)
#include "yaml-cpp/yaml.h"
#pragma warning(pop)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#define throw (void) /* really? */
#include "yaml-cpp/yaml.h"
#pragma GCC diagnostic pop
#endif

namespace YAML {
	template<>
	struct convert<CinMath::Vector2> final
	{
		static Node encode(const CinMath::Vector2& value) 
		{
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CinMath::Vector2& value)
		{
			if (!node.IsSequence() or node.size() != 2U)
				return false;

			value.x = node[0U].as<float>();
			value.y = node[1U].as<float>();

			return true;
		}
	};

	template<>
	struct convert<CinMath::Vector3> final
	{
		static Node encode(const CinMath::Vector3& value)
		{
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			node.push_back(value.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CinMath::Vector3& value)
		{
			if (!node.IsSequence() or node.size() != 3U)
				return false;

			value.x = node[0U].as<float>();
			value.y = node[1U].as<float>();
			value.z = node[2U].as<float>();

			return true;
		}
	};

	template<>
	struct convert<CinMath::Vector4> final
	{
		static Node encode(const CinMath::Vector4& value)
		{
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			node.push_back(value.z);
			node.push_back(value.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, CinMath::Vector4& value)
		{
			if (!node.IsSequence() or node.size() != 4U)
				return false;

			value.x = node[0U].as<float>();
			value.y = node[1U].as<float>();
			value.z = node[2U].as<float>();
			value.w = node[3U].as<float>();

			return true;
		}
	};
}

YAML::Emitter& operator<<(YAML::Emitter& out, const CinMath::Vector2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const CinMath::Vector3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const CinMath::Vector4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::STL::Filepath& filepath)
{
	return (out << filepath.string());
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::ETextureSamplerFilterMode filterMode)
{
	return (out << static_cast<uint32_t>(filterMode));
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::ETextureSamplerWrapMode wrapMode)
{
	return (out << static_cast<uint32_t>(wrapMode));
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::EBodyType bodyType)
{
	return (out << static_cast<uint32_t>(bodyType));
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::TagComponent& tagComponent)
{
	out << YAML::Key << "TagComponent";
	out << YAML::BeginMap;
	out << YAML::Key << "Tag" << YAML::Value << tagComponent.Tag;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::TransformComponent& transformComponent)
{
	out << YAML::Key << "TransformComponent";
	out << YAML::BeginMap;
	out << YAML::Key << "Translation" << YAML::Value << transformComponent.Translation;
	out << YAML::Key << "Scale" << YAML::Value << transformComponent.Scale;
	out << YAML::Key << "Rotation" << YAML::Value << transformComponent.Rotation;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::SpriteRendererComponent& spriteRendererComponent)
{
	out << YAML::Key << "SpriteRendererComponent";
	out << YAML::BeginMap;
	if (const Cinnamon::Texture2D* const texture{ spriteRendererComponent.Texture })
	{
		const auto& textureSpecification{ spriteRendererComponent.Texture->GetSpecification() };

		out << YAML::Key << "Texture" << YAML::Value << texture->GetAssetPath();
		out << YAML::Key << "Filter" << YAML::Value << textureSpecification.SamplerFilterMode;
		out << YAML::Key << "Wrap" << YAML::Value << textureSpecification.SamplerWrapMode;
	}
	else
	{
		out << YAML::Key << "Texture" << YAML::Value << Cinnamon::STL::Filepath("");
		out << YAML::Key << "Filter" << YAML::Value << Cinnamon::ETextureSamplerFilterMode::Linear;
		out << YAML::Key << "Wrap" << YAML::Value << Cinnamon::ETextureSamplerWrapMode::Repeat;
	}

	out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
	out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::PointLightComponent& pointLightComponent)
{
	out << YAML::Key << "PointLightComponent";
	out << YAML::BeginMap;
	out << YAML::Key << "Color" << YAML::Value << pointLightComponent.Color;
	out << YAML::Key << "Intensity" << YAML::Value << pointLightComponent.Intensity;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::RigidBody2DComponent& rigidBodyComponent)
{
	out << YAML::Key << "RigidBody2DComponent";
	out << YAML::BeginMap;
	out << YAML::Key << "BodyType" << YAML::Value << rigidBodyComponent.BodyType;
	out << YAML::Key << "Offset" << YAML::Value << rigidBodyComponent.Offset;
	out << YAML::Key << "Angle" << YAML::Value << rigidBodyComponent.Angle;
	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Cinnamon::Box2DColliderComponent& box2DCollider)
{
	CIN_UNUSED(box2DCollider);

	out << YAML::Key << "Box2DColliderComponent";
	out << YAML::BeginMap;
	out << YAML::Key << "Size" << YAML::Value << box2DCollider.Size;
	out << YAML::EndMap;
	return out;
}

namespace Cinnamon {
	SceneSerializer::SceneSerializer(Scene* const scene, const STL::Unique<AssetManager>& assetManager) noexcept
		:
		m_Scene(scene),
		m_AssetManager(assetManager)
	{
		CIN_ASSERT(scene && assetManager);
	}

	Errr SceneSerializer::operator<<(const STL::Filepath& filepath)
	{
		return Deserialize(filepath);
	}

	Errr SceneSerializer::operator>>(const STL::Filepath& filepath)
	{
		return Serialize(filepath);
	}

	Errr SceneSerializer::Serialize(const STL::Filepath& filepath)
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;

		emitter << YAML::Key << "Scene" << YAML::Value << filepath.string();
		emitter << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (const ECS::EntityID entityID : ECS::View(m_Scene->GetRegistry()))
			SerializeEntity(emitter, Entity{ entityID, m_Scene });

		emitter << YAML::EndSeq;
		emitter << YAML::EndMap;

		std::ofstream output(filepath);
		if (!output)
		{
			CIN_ERROR("Failed to open scene file with path: {}", filepath.string());
			return Error::Failure;
		}

		output.write(emitter.c_str(), emitter.size());
		output.close();

		return Error::Success;
	}

	Errr SceneSerializer::Deserialize(const STL::Filepath& filepath)
	{
		YAML::Node data{ YAML::LoadFile(filepath.string()) };
		if (!data["Scene"])
		{
			CIN_ERROR("{} is not a valid scene file: Couldn't locate scene header in scene file", filepath.string());
			return Error::Failure;
		}
		
		const STL::String scenePathQuoted{ data["Scene"].as<STL::String>() };
		const auto entities{ data["Entities"] };
		if (entities)
		{
			for (const auto iterator : entities)
			{
				const auto tagComponent{ iterator["TagComponent"] };
				if (!tagComponent)
					continue;

				const auto entityTag{ tagComponent["Tag"].as<STL::String>() };
				Entity deserializedEntity{ m_Scene->CreateEntity(entityTag) };

				const auto transform{ iterator["TransformComponent"] };
				if(transform)
				{
					auto& transformComponent{ deserializedEntity.GetComponent<TransformComponent>() };

					transformComponent.Translation	= transform["Translation"].as<CinMath::Vector3>();
					transformComponent.Scale		= transform["Scale"].as<CinMath::Vector3>();
					transformComponent.Rotation		= transform["Rotation"].as<CinMath::Vector3>();
				}

				const auto spriteRenderer{ iterator["SpriteRendererComponent"] };
				if (spriteRenderer)
				{
					auto& spriteRendererComponent	{ deserializedEntity.AddComponent<SpriteRendererComponent>() };
					
					const auto texturePath			{ spriteRenderer["Texture"].as<STL::String>() };
					const auto filterMode			{ static_cast<ETextureSamplerFilterMode>(spriteRenderer["Filter"].as<uint32_t>()) };
					const auto wrapMode				{ static_cast<ETextureSamplerWrapMode>(spriteRenderer["Wrap"].as<uint32_t>()) };
					const auto textureSpecification	{ TextureSpecification{ wrapMode, filterMode } };

					spriteRendererComponent.Texture			= std::filesystem::exists(texturePath) ? m_AssetManager->LoadTexture(texturePath, textureSpecification) : nullptr;
					spriteRendererComponent.Color			= spriteRenderer["Color"].as<CinMath::Vector4>();
					spriteRendererComponent.TilingFactor	= spriteRenderer["TilingFactor"].as<float>();
				}

				const auto pointLight{ iterator["PointLightComponent"] };
				if (pointLight)
				{
					auto& pointLightComponent{ deserializedEntity.AddComponent<PointLightComponent>() };
					pointLightComponent.Color = pointLight["Color"].as<CinMath::Vector4>();
					pointLightComponent.Intensity = pointLight["Intensity"].as<float>();
				}

				const auto rigidBody2D{ iterator["RigidBody2DComponent"] };
				if (rigidBody2D)
				{
					auto& rigidBody2DComponent{ deserializedEntity.AddComponent<RigidBody2DComponent>() };
					rigidBody2DComponent.BodyType	= static_cast<EBodyType>(rigidBody2D["BodyType"].as<uint32_t>());
					rigidBody2DComponent.Offset		= rigidBody2D["Offset"].as<CinMath::Vector2>();
					rigidBody2DComponent.Angle		= rigidBody2D["Angle"].as<float>();
				}

				const auto box2DCollider{ iterator["Box2DColliderComponent"] };
				if (box2DCollider)
				{
					auto& box2DColliderComponent{ deserializedEntity.AddComponent<Box2DColliderComponent>() };
					box2DColliderComponent.Size = box2DCollider["Size"].as<CinMath::Vector2>();
				}
			}
		}

		return Error::Success;
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& emitter, const Entity entity)
	{
		emitter << YAML::BeginMap; /* Entity */
			
		if (entity.HasComponent<TagComponent>())
			emitter << entity.GetComponent<TagComponent>();
		
		if (entity.HasComponent<TransformComponent>())
			emitter << entity.GetComponent<TransformComponent>();

		if (entity.HasComponent<SpriteRendererComponent>())
			emitter << entity.GetComponent<SpriteRendererComponent>();

		if (entity.HasComponent<PointLightComponent>())
			emitter << entity.GetComponent<PointLightComponent>();

		if (entity.HasComponent<RigidBody2DComponent>())
			emitter << entity.GetComponent<RigidBody2DComponent>();

		if (entity.HasComponent<Box2DColliderComponent>())
			emitter << entity.GetComponent<Box2DColliderComponent>();

		emitter << YAML::EndMap; /* Entity */
	}
}