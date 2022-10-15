#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.h"

class SceneHierarchyPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(SceneHierarchyPanel)
public:
	constexpr explicit SceneHierarchyPanel() noexcept = default;
	constexpr virtual ~SceneHierarchyPanel() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
};