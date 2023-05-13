#include "Cinnamon/include/Core/Layer.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

namespace Cinnamon {
	class Renderer;
	class Window;
	class KeyPressedEvent;
	class Scene;
}

class EditorLayer final : public Cinnamon::Layer
{
private:
	NON_COPYABLE(EditorLayer)
public:
	explicit EditorLayer(
		const Cinnamon::STL::Unique<Cinnamon::Window>& window,
		const Cinnamon::STL::Unique<Cinnamon::Renderer>& renderer) noexcept;
	
	virtual ~EditorLayer() noexcept;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;

	virtual void OnEvent(const Cinnamon::Event& event) override final;
private:
	bool OnKeyPressed(const Cinnamon::KeyPressedEvent& event);
private:
	const Cinnamon::STL::Unique<Cinnamon::Window>& m_Window;
	const Cinnamon::STL::Unique<Cinnamon::Renderer>& m_Renderer;

	Cinnamon::Scene* m_SceneContext;
	Cinnamon::Entity m_SelectionContext;
	Cinnamon::STL::Vector<EditorPanelBase*> m_Panels;
};