#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

EditorPanelBase::EditorPanelBase(
	ProjectContext		projectContext,
	SceneContext		sceneContext, 
	SelectionContext	selectionContext) noexcept
	:
	m_ProjectContext(projectContext),
	m_SceneContext(sceneContext),
	m_SelectionContext(selectionContext)
{}