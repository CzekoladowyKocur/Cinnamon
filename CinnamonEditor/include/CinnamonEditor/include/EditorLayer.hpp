#include "Cinnamon/include/Core/Layer.hpp"
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

namespace Cinnamon {
	class Renderer;
	class Window;
	class KeyPressedEvent;
}

class EditorLayer final : public Cinnamon::Layer
{
private:
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

	Cinnamon::STL::Vector<EditorPanelBase*> m_Panels;
};