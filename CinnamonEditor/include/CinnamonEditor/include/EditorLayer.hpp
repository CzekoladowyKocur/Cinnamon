#include "Cinnamon/include/Core/Layer.hpp"
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"

namespace Cinnamon {
	class KeyPressedEvent;
}

class EditorLayer final : public Cinnamon::Layer
{
private:
public:
	explicit EditorLayer() noexcept = default;
	virtual ~EditorLayer() noexcept = default;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;

	virtual void OnEvent(const Cinnamon::Event& event) override final;
private:
	bool OnKeyPressed(const Cinnamon::KeyPressedEvent& event);
private:
	Cinnamon::STL::Vector<EditorPanelBase*> m_Panels;
};