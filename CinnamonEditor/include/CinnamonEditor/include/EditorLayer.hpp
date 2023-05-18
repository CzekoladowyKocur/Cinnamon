#include "Cinnamon/include/Core/Layer.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

namespace Cinnamon {
	class Window;
	class Renderer;
	class AssetManager;
	class KeyPressedEvent;
	class Scene;
}

class Project;
struct EditorSettings;

class EditorLayer final : public Cinnamon::Layer
{
private:
	NON_COPYABLE(EditorLayer)
public:
	explicit EditorLayer(
		EditorSettings& editorSettings,
		const Cinnamon::STL::Unique<Cinnamon::Window>& window,
		const Cinnamon::STL::Unique<Cinnamon::Renderer>& renderer,
		const Cinnamon::STL::Unique<Cinnamon::AssetManager>& assetManager) noexcept;
	
	virtual ~EditorLayer() noexcept;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;

	virtual void OnEvent(const Cinnamon::Event& event) override final;
private:
	bool OnKeyPressed(const Cinnamon::KeyPressedEvent& event);

	void NewProject();
	void OpenProject();
	void SaveProject();

	void NewScene();
	void OpenScene();
	void SaveScene();
	void SaveSceneAs();
	/* Popups */
	void NewProjectPopup();
	void NewScenePopup();

	void EmptyScene();
	void UpdateWindowTitle();
private:
	EditorSettings& m_EditorSettings;
	Project* m_Project;

	const Cinnamon::STL::Unique<Cinnamon::Window>& m_Window;
	const Cinnamon::STL::Unique<Cinnamon::Renderer>& m_Renderer;
	const Cinnamon::STL::Unique<Cinnamon::AssetManager>& m_AssetManager;

	Cinnamon::Scene* m_SceneContext;
	Cinnamon::Entity m_SelectionContext;
	Cinnamon::STL::Filepath m_CurrentScenePath;
	Cinnamon::STL::Vector<EditorPanelBase*> m_Panels;

	bool m_OpenNewProjectPopup;
	bool m_OpenNewScenePopup;

	static constexpr const char* s_CinnamonProjectExtension{ ".cinproj" };
	static constexpr const char* s_CinnamonProjectExtensionFilter{ "Cinnamon Project (*.cinproj)\0*.cinproj\0" };
	static constexpr const char* s_CinnamonSceneExtension{ ".cinscene" };
	static constexpr const char* s_CinnamonSceneExtensionFilter{ "Cinnamon Scene (*.cinscene)\0*.cinscene\0" };
};