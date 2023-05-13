#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"

class EntityPropertiesPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(EntityPropertiesPanel)
public:
	explicit EntityPropertiesPanel(Cinnamon::Scene*& sceneContext, Cinnamon::Entity& selectionContext) noexcept;
	virtual ~EntityPropertiesPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
	void DrawEntityComponents(Cinnamon::Entity entity);

	bool BeginCollapsingHeader(const Cinnamon::STL::StringView label);
	void EndCollapsingHeader();
	void DrawCollapsingHeaderRow(const Cinnamon::STL::StringView label, const std::function<void()> functor);
};