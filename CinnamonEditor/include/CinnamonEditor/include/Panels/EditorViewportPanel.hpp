#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "CinnamonEditor/include/EditorCamera.hpp"
#include "CinnamonEditor/include/Gizmo.hpp"

namespace Cinnamon {
	class Renderer;
	class SceneRenderer;
	class AssetManager;
	class KeyPressedEvent;
	class MousePressedEvent;
	class WindowResizedEvent;
}

class Project;

class EditorViewportPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(EditorViewportPanel)
public:
	explicit EditorViewportPanel(
		Project*& projectContext,
		Cinnamon::Scene*& sceneContext,
		Cinnamon::Entity& selectionContext,
		const Cinnamon::STL::Unique<Cinnamon::Renderer>& renderer, 
		const Cinnamon::STL::Unique<Cinnamon::AssetManager>& assetManager, 
		const uint32_t viewportWidth, 
		const uint32_t viewportHeight) noexcept;
	
	virtual ~EditorViewportPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;
	
	constexpr virtual const char* GetPanelName() const override final;
private:
	bool OnKeyPressed(const Cinnamon::KeyPressedEvent& event);
	bool OnMousePressed(const Cinnamon::MousePressedEvent& event);
	bool OnWindowResized(const Cinnamon::WindowResizedEvent& event);

	void RenderToolbar();
	void RenderViewport();
private:
	EditorCamera m_EditorCamera;
	EGizmoOperation m_GizmoOperation;
	
	const Cinnamon::STL::Unique<Cinnamon::Renderer>& m_Renderer;
	const Cinnamon::STL::Unique<Cinnamon::AssetManager>& m_AssetManager;

	Cinnamon::STL::Unique<Cinnamon::SceneRenderer> m_SceneRenderer;

	struct
	{
		float AspectRatio;
		float Width;
		float Height;
		bool Focused;
		bool Hovered;

		CinMath::Vector2 BoundsX;
		CinMath::Vector2 BoundsY;
	} m_Viewport;
};