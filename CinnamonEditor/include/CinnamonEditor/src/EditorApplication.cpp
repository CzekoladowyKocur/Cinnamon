#include "CinnamonEditor/include/EditorApplication.h"
#include "CinnamonEditor/include/EditorLayer.h"
#include "Cinnamon/include/Core/Logger.h"

using namespace Cinnamon;
EditorApplication::EditorApplication() noexcept
	:
	Application("Cinnamon Editor", 800U, 600U, false),
	m_EditorLayer(cinew EditorLayer)
{}

EditorApplication::~EditorApplication() noexcept
{
	CIN_ASSERT(m_EditorLayer);
	cindel m_EditorLayer;
}

Errr EditorApplication::OnUserInitialize()
{
	PushLayer(m_EditorLayer);
	CIN_TRACE("Pushed editor application layer");

	return Error::Success;
}

void EditorApplication::OnUserShutdown()
{
	PopLayer(m_EditorLayer);
	CIN_TRACE("Popped editor application layer");
}

REGISTER_CINNAMON_APPLICATION(EditorApplication);
#include "Cinnamon/include/Core/EntryPoint.h"