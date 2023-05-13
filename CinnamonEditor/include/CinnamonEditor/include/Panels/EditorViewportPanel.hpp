#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

namespace Cinnamon {
	class Renderer;
	class SceneRenderer;
}

class EditorViewportPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(EditorViewportPanel)
public:
	explicit EditorViewportPanel(
		Cinnamon::Scene*& sceneContext,
		Cinnamon::Entity& selectionContext,
		const Cinnamon::STL::Unique<Cinnamon::Renderer>& renderer, 
		const uint32_t viewportWidth, 
		const uint32_t viewportHeight) noexcept;
	
	virtual ~EditorViewportPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;
	
	constexpr virtual const char* GetPanelName() const override final;
private:
	const Cinnamon::STL::Unique<Cinnamon::Renderer>& m_Renderer;

	Cinnamon::STL::Unique<Cinnamon::SceneRenderer> m_SceneRenderer;
};