#include "CinnamonEditor/include/EditorApplication.h"
#include "CinnamonEditor/include/EditorLayer.h"
#include "Cinnamon/include/Core/Logger.h"

using namespace Cinnamon;
EditorApplication::EditorApplication() noexcept
	:
	Application(),
	m_EditorLayer(cinew EditorLayer)
{}

EditorApplication::~EditorApplication() noexcept
{
	CIN_ASSERT(m_EditorLayer);
	cindel m_EditorLayer;
}

bool EditorApplication::OnUserInitialize()
{
	PushLayer(m_EditorLayer);
	CIN_TRACE("Pushed editor application layer");

	return true;
}

bool EditorApplication::OnUserShutdown()
{
	PopLayer(m_EditorLayer);
	CIN_TRACE("Popped editor application layer");

	return true;
}

REGISTER_CINNAMON_APPLICATION(EditorApplication);
#include "Cinnamon/include/Core/EntryPoint.h"