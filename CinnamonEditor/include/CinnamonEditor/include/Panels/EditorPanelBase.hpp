#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Scene;
	class Entity;
	class Event;
}

class EditorPanelBase
{
private:
	NON_COPYABLE(EditorPanelBase)
public:
	explicit EditorPanelBase(Cinnamon::Scene*& sceneContext, Cinnamon::Entity& selectionContext) noexcept;
	virtual ~EditorPanelBase() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) = 0;
	virtual void OnGUIRender() = 0;
	virtual void OnEvent(const Cinnamon::Event& event) = 0;

	constexpr virtual const char* GetPanelName() const = 0;
public:
	Cinnamon::Scene*& m_SceneContext;
	Cinnamon::Entity& m_SelectionContext;
};