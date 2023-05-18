#include "CinnamonEditor/include/EditorCamera.hpp"
#include "Cinnamon/include/Event/Event.hpp"
#include "Cinnamon/include/Event/MouseEvent.hpp"
#include "Cinnamon/include/Core/Input.hpp"

using namespace Cinnamon;
EditorCamera::EditorCamera(const float aspectRatio) noexcept
	:
	m_AspectRatio(aspectRatio),
	m_ViewMatrix(1.0f),
	m_ProjectionMatrix(1.0f),
	m_OrthographicProjection
	{
		.Scale{ 5.0f },
		.Near{ -1.0f },
		.Far{ 1.0f }
	},
	m_Position(0.0f),
	m_CachedMousePosition(CinMath::Vector2{ static_cast<float>(Input::GetMousePositionX()), static_cast<float>(Input::GetMousePositionY()) })
{
	RecalculateProjection();
}

void EditorCamera::RecalculateProjection()
{
	const float orthoLeft = -m_OrthographicProjection.Scale * m_AspectRatio * 0.5f;
	const float orthoRight = m_OrthographicProjection.Scale * m_AspectRatio * 0.5f;
	const float orthoBottom = -m_OrthographicProjection.Scale * 0.5f;
	const float orthoTop = m_OrthographicProjection.Scale * 0.5f;

	m_ViewMatrix = CinMath::TranslateIdentity<4, 4, float>(m_Position);
	m_ProjectionMatrix = CinMath::OrthographicProjection(orthoLeft, orthoRight, orthoBottom, orthoTop);
}

bool EditorCamera::OnMouseMoved(const Cinnamon::MouseMovedEvent& event, const bool shouldUpdate)
{
	const auto [xPosition, yPosition] { event.GetPosition() };

	constexpr float g_MoveSpeed{ 0.005f };
	const CinMath::Vector2 delta
	{
		m_CachedMousePosition.x - static_cast<float>(xPosition),
		m_CachedMousePosition.y - static_cast<float>(yPosition)
	};

	m_CachedMousePosition.x = static_cast<float>(xPosition);
	m_CachedMousePosition.y = static_cast<float>(yPosition);
	
	if (shouldUpdate and Input::IsKeyPressed(Key::LeftAlt) and Input::IsMouseButtonPressed(Mouse::LeftButton))
	{
		m_Position.x -= delta.x * g_MoveSpeed * (m_OrthographicProjection.Scale * 0.33f);
		m_Position.y += delta.y * g_MoveSpeed * (m_OrthographicProjection.Scale * 0.33f);
	}
	
	return false;
}

bool EditorCamera::OnMouseScrolled(const Cinnamon::MouseScrolledEvent& event, const bool shouldUpdate)
{
	if (shouldUpdate)
	{
		m_OrthographicProjection.Scale -= event.GetVerticalDelta() * 0.5f;
		m_OrthographicProjection.Scale = CIN_CLAMP(m_OrthographicProjection.Scale, 0.1f, 100.0f);
	}

	return false;
}

void EditorCamera::OnEvent(const Event& event, const bool shouldUpdate)
{
	const EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseMovedEvent>(std::bind(&EditorCamera::OnMouseMoved, this, std::placeholders::_1, std::placeholders::_2), shouldUpdate);
	dispatcher.Dispatch<MouseScrolledEvent>(std::bind(&EditorCamera::OnMouseScrolled, this, std::placeholders::_1, std::placeholders::_2), shouldUpdate);
}

void EditorCamera::SetAspectRatio(const float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	RecalculateProjection();
}

CinMath::Matrix4 EditorCamera::GetViewMatrix() const
{
	return m_ViewMatrix;
}

CinMath::Matrix4 EditorCamera::GetProjectionMatrix() const
{
	return m_ProjectionMatrix;
}

CinMath::Matrix4 EditorCamera::GetViewProjectionMatrix() const
{
	return m_ProjectionMatrix * m_ViewMatrix;
}