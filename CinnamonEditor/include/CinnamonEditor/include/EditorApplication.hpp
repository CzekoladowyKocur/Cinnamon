#pragma once
#include "Cinnamon/include/Core/Application.hpp"
#include "CinnamonEditor/include/EditorSettings.hpp"

namespace Cinnamon {
	class Window;
	class Renderer;
	class GUIRenderer;
	class AssetManager;
	class WindowResizedEvent;
	class ApplicationRenderEvent;
}

class EditorApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(EditorApplication)
public:
	explicit EditorApplication() noexcept;
	virtual ~EditorApplication() noexcept;

	virtual Errr OnUserInitialize() final override;
	virtual void OnUserShutdown() final override;
	
	virtual void OnEvent(const Cinnamon::Event& event) final override;
private:
	bool OnWindowResized(const Cinnamon::WindowResizedEvent& event);
	bool OnApplicationRender(const Cinnamon::ApplicationRenderEvent& event);

	[[nodiscard]] EditorSettings LoadLocalEditorSettings();
	void SaveLocalEditorSettings(const EditorSettings& settings);
private:
	EditorSettings m_Settings;
	Cinnamon::STL::Unique<Cinnamon::Renderer> m_Renderer;
	Cinnamon::STL::Unique<Cinnamon::GUIRenderer> m_GUIRenderer;
	Cinnamon::STL::Unique<Cinnamon::AssetManager> m_AssetManager;
	Cinnamon::Layer* m_EditorLayer;
};