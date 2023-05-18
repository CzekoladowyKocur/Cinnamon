#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class Texture2D;
	
	class Environment final
	{
	private:
		NON_COPYABLE(Environment)
	public:
		explicit Environment() noexcept;
		~Environment() noexcept;

		void SetAmbientLightColor(const CinMath::Vector3& color);
		void SetBackgroundTexture(Texture2D* texture);

		[[nodiscard]] CinMath::Vector3&
			GetAmbientLightColor();

		[[nodiscard]] const CinMath::Vector3&
			GetAmbientLightColor() const;

		[[nodiscard]] const Texture2D*
			GetBackgroundTexture() const;

		[[nodiscard]] Texture2D*
			GetBackgroundTexture();
	private:
		CinMath::Vector3	m_AmbientLightColor;
		Texture2D*			m_BackgroundTexture;
	};
}