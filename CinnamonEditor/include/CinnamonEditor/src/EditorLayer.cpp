#include "CinnamonEditor/include/EditorLayer.h"
#include "Cinnamon/include/Core/Logger.h"

void EditorLayer::OnAttach()
{}

void EditorLayer::OnUpdate(const Timestep timestep)
{
#ifdef CIN_DEBUG
	CIN_WARN("Editor layer update timestep: {}", timestep);
#else
	CIN_UNUSED(timestep);
#endif
}

void EditorLayer::OnDetach()
{}
