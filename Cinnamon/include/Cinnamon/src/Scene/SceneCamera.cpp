#include "Cinnamon/include/Scene/SceneCamera.hpp"

namespace Cinnamon {
	SceneCamera::SceneCamera(const float apsectRatio) noexcept
		:
		m_ViewProjection(CinMath::Matrix4::Identity()),
		m_AspectRatio(apsectRatio),
		m_OrthographicProjection
		{
			.Scale{ 5.0f }
		}
	{
		RecalculateProjection();
	}

	SceneCamera::~SceneCamera()
	{}

	void SceneCamera::RecalculateProjection()
	{
		const float orthoLeft{ -m_OrthographicProjection.Scale * m_AspectRatio * 0.5f };
		const float orthoRight{ m_OrthographicProjection.Scale * m_AspectRatio * 0.5f };
		const float orthoBottom{ -m_OrthographicProjection.Scale * 0.5f };
		const float orthoTop{ m_OrthographicProjection.Scale * 0.5f };

		m_ViewProjection = CinMath::OrthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop);
	}

	void SceneCamera::SetAspectRatio(const float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		RecalculateProjection();
	}

	void SceneCamera::SetOrthographicProjection(
		const float aspectRatio,
		const float scale)
	{
		m_AspectRatio = aspectRatio;
		m_OrthographicProjection.Scale = scale;
		RecalculateProjection();
	}

	float SceneCamera::GetAspectRatio() const
	{
		return m_AspectRatio;
	}

	const CinMath::Matrix4& SceneCamera::GetViewProjection() const
	{
		return m_ViewProjection;
	}
}