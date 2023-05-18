#include "Cinnamon/include/Scene/Environment.hpp"

namespace Cinnamon {
	Environment::Environment() noexcept
		:
		m_AmbientLightColor(0.5f, 0.5f, 0.5f),
		m_BackgroundTexture(nullptr)
	{}

	Environment::~Environment() noexcept
	{}

	void Environment::SetAmbientLightColor(const CinMath::Vector3& color)
	{
		m_AmbientLightColor = color;
	}

	void Environment::SetBackgroundTexture(Texture2D* texture)
	{
		m_BackgroundTexture = texture;
	}

	CinMath::Vector3& Environment::GetAmbientLightColor()
	{
		return m_AmbientLightColor;
	}

	const CinMath::Vector3& Environment::GetAmbientLightColor() const
	{
		return m_AmbientLightColor;
	}

	const Texture2D* Environment::GetBackgroundTexture() const
	{
		return m_BackgroundTexture;
	}

	Texture2D* Environment::GetBackgroundTexture()
	{
		return m_BackgroundTexture;
	}
}