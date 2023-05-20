#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Scene/SceneCamera.hpp"
#include "Cinnamon/include/Physics/PhysicsBody2D.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class Texture2D;

	struct TagComponent final
	{
		constexpr explicit TagComponent(const STL::StringView tag) noexcept
			:
			Tag(tag)
		{}

		constexpr operator const STL::String& () const noexcept
		{
			return Tag;
		}

		constexpr operator STL::String &() noexcept
		{
			return Tag;
		}

		STL::String Tag;
	};

	struct TransformComponent final
	{
		constexpr explicit TransformComponent() noexcept
			:
			Translation(),
			Scale(1.0f, 1.0f, 1.0f),
			Rotation()
		{}

		constexpr explicit TransformComponent(const CinMath::Vector3& translation) noexcept
			:
			Translation(translation)
		{}

		CinMath::Matrix4 Calculate() const noexcept
		{
			return
					CinMath::TranslateIdentity<4U, 4U, float>(Translation)													* 
					CinMath::RotateZ(CinMath::Matrix4::Identity(), CinMath::Angle(CinMath::Degrees::FromValue(Rotation.z))) *
					CinMath::Scale(CinMath::Matrix4(1.0), Scale);
		}

		CinMath::Vector3 Translation;
		CinMath::Vector3 Scale;
		CinMath::Vector3 Rotation;
	};

	struct CameraComponent final
	{
		SceneCamera Camera;
		bool Primary;
	};

	struct SpriteRendererComponent final
	{
		constexpr explicit SpriteRendererComponent() noexcept
			:
			Texture(nullptr),
			Color(1.0f, 1.0f, 1.0f, 1.0f),
			TilingFactor(1.0f)
		{}

		Texture2D* Texture;
		CinMath::Vector4 Color;
		float TilingFactor;
	};

	struct PointLightComponent final
	{
		CinMath::Vector4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Intensity{ 1.0f };
	};

	struct RigidBody2DComponent final
	{
		RuntimeBodyHandle	RuntimeBody{ nullptr };
		EBodyType			BodyType{ EBodyType::Static };
		CinMath::Vector2	Offset{ 0.0f };
		float				Angle{ 0.0f };
	};

	struct Box2DColliderComponent final
	{
		CinMath::Vector2	Size{ 1.0f };
	};
}