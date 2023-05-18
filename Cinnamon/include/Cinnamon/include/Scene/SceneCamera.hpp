#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class SceneCamera final
	{
	public:
		explicit SceneCamera(const float aspectRatio) noexcept;
		~SceneCamera();
		
		void SetAspectRatio(const float aspectRatio);
		
		void SetOrthographicProjection(
			const float aspectRatio,
			const float scale);

		[[nodiscard]] float
			GetAspectRatio() const;

		[[nodiscard]] const CinMath::Matrix4& 
			GetViewProjection() const;
	private:
		void RecalculateProjection();
	private:
		CinMath::Matrix4 m_ViewProjection;
		float m_AspectRatio;

		struct
		{
			float Scale;
		} m_OrthographicProjection;
	};
}