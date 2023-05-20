#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Scene;
	class Entity;
	class Event;
}

class Project;
using ProjectContext	= Project*&;
using SceneContext		= Cinnamon::Scene*&;
using SelectionContext	= Cinnamon::Entity&;

class EditorPanelBase
{
private:
	NON_COPYABLE(EditorPanelBase)
public:
	explicit EditorPanelBase(
		ProjectContext		projectContext,
		SceneContext		sceneContext, 
		SelectionContext	selectionContext) noexcept;

	virtual ~EditorPanelBase() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) = 0;
	virtual void OnGUIRender() = 0;
	virtual void OnEvent(const Cinnamon::Event& event) = 0;

	constexpr virtual const char* GetPanelName() const = 0;
protected:
	ProjectContext		m_ProjectContext;
	SceneContext		m_SceneContext;
	SelectionContext	m_SelectionContext;
};