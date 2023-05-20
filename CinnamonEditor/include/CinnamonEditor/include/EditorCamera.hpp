#pragma once
#include "CinMath/CinMath.h"

namespace Cinnamon {
	class Event;
	class MouseMovedEvent;
	class MouseScrolledEvent;
}

class EditorCamera final
{
public:
	explicit EditorCamera(const float aspectRatio) noexcept;
	~EditorCamera() = default;
	
	void RecalculateProjection();
	void OnEvent(const Cinnamon::Event& event, const bool shouldUpdate);
	void SetAspectRatio(const float aspectRatio);

	[[nodiscard]] CinMath::Vector3 GetPosition() const;
	[[nodiscard]] CinMath::Matrix4 GetViewMatrix() const;
	[[nodiscard]] CinMath::Matrix4 GetProjectionMatrix() const;
	[[nodiscard]] CinMath::Matrix4 GetViewProjectionMatrix() const;
private:
	bool OnMouseMoved(const Cinnamon::MouseMovedEvent& event, const bool shouldUpdate);
	bool OnMouseScrolled(const Cinnamon::MouseScrolledEvent& event, const bool shouldUpdate);
private:
	CinMath::Matrix4 m_ViewMatrix;
	CinMath::Matrix4 m_ProjectionMatrix;
	float m_AspectRatio;

	struct
	{
		float Scale;
		float Near;
		float Far;
	} m_OrthographicProjection;
	/* Controls */
	CinMath::Vector3 m_Position;
	CinMath::Vector2 m_CachedMousePosition;
};
