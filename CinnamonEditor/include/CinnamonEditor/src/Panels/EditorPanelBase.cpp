#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

using namespace Cinnamon;
EditorPanelBase::EditorPanelBase(Scene*& sceneContext, Entity& selectionContext) noexcept
	:
	m_SceneContext(sceneContext),
	m_SelectionContext(selectionContext)
{}