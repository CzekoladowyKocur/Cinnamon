#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

class EditorViewportPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(EditorViewportPanel)
public:
	constexpr explicit EditorViewportPanel() noexcept = default;
	constexpr virtual ~EditorViewportPanel() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;
	
	constexpr virtual const char* GetPanelName() const override final;
private:
};