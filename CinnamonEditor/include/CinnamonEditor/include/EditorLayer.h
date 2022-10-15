#include "Cinnamon/include/Core/Layer.h"
#include "CinnamonEditor/include/Panels/EditorPanelBase.h"

class EditorLayer final : public Cinnamon::Layer
{
private:
public:
	constexpr explicit EditorLayer() noexcept = default;
	constexpr virtual ~EditorLayer() noexcept = default;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;
private:
	Cinnamon::STL::Vector<EditorPanelBase*> m_Panels;
};