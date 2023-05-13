#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
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

		constexpr operator const char* () const noexcept
		{
			return Tag.data();
		}

		STL::String Tag;
	};

	struct TransformComponent final
	{
		constexpr explicit TransformComponent() noexcept
			:
			Position()
		{}

		constexpr explicit TransformComponent(const CinMath::Vector3& position) noexcept
			:
			Position(position)
		{}

		CinMath::Matrix4 Calculate() const noexcept
		{
			return CinMath::TranslateIdentity<4U, 4U, float>(Position);
		}

		CinMath::Vector3 Position;
	};
}