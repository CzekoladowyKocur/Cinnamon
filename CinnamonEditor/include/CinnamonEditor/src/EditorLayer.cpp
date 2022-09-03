#include "CinnamonEditor/include/EditorLayer.h"
#include "Cinnamon/include/Core/Logger.h"
#include "Cinnamon/include/Core/Input.h"

void EditorLayer::OnAttach()
{}

void EditorLayer::OnUpdate(const Timestep timestep)
{
#ifdef CIN_DEBUG
	FunctionVariable struct {
		uint32_t x{ 0U }, y{ 0U };
	} f_CachedMousePosition;

	const auto [currentMousePositionX, currentMousePositionY] { Cinnamon::Input::GetMousePosition() };
	if (currentMousePositionX != f_CachedMousePosition.x || currentMousePositionY != f_CachedMousePosition.y)
	{
		CIN_TRACE("Mouse moved [new x, new y]: {0}, {1}", currentMousePositionX, currentMousePositionY);
		f_CachedMousePosition.x = currentMousePositionX;
		f_CachedMousePosition.y = currentMousePositionY;
	}

	if (Cinnamon::Input::IsMouseButtonPressed(Cinnamon::Mouse::LeftButton))
		CIN_TRACE("Pressed left mouse button");
	else if (Cinnamon::Input::IsMouseButtonPressed(Cinnamon::Mouse::MiddleButton))
		CIN_TRACE("Pressed middle mouse button");
	else if(Cinnamon::Input::IsMouseButtonPressed(Cinnamon::Mouse::RightButton))
		CIN_TRACE("Pressed right mouse button");

	for (uint32_t i{ 0U }; i < static_cast<uint32_t>(Cinnamon::Key::KeysEnd); ++i)
		if (Cinnamon::Input::IsKeyPressed(static_cast<Cinnamon::Key>(i)))
			CIN_TRACE("Pressed key: {}", Cinnamon::KeyToString(static_cast<Cinnamon::Key>(i)));
	
	//CIN_WARN("Editor layer update timestep: {}", timestep);
	CIN_UNUSED(timestep);
#else
	CIN_UNUSED(timestep);
#endif
}

void EditorLayer::OnDetach()
{}
