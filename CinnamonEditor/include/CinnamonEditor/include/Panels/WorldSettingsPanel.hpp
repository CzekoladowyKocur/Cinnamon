#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

class WorldSettingsPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(WorldSettingsPanel)
public:
	explicit WorldSettingsPanel(
		ProjectContext		projectContext,
		SceneContext		sceneContext,
		SelectionContext	selectionContext) noexcept;

	virtual ~WorldSettingsPanel() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) final override;
	virtual void OnGUIRender() final override;
	virtual void OnEvent(const Cinnamon::Event& event) final override;
	constexpr virtual const char* GetPanelName() const final override;
private:
};