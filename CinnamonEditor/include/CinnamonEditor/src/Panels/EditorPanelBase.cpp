#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

using namespace Cinnamon;
EditorPanelBase::EditorPanelBase(
	Project*& projectContext,
	Scene*& sceneContext, 
	Entity& selectionContext) noexcept
	:
	m_ProjectContext(projectContext),
	m_SceneContext(sceneContext),
	m_SelectionContext(selectionContext)
{}