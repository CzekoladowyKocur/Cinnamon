#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"

class Project;

class SceneHierarchyPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(SceneHierarchyPanel)
public:
	explicit SceneHierarchyPanel(
		Project*& projectContext,
		Cinnamon::Scene*& sceneContext, 
		Cinnamon::Entity& selectionContext) noexcept;
	
	virtual ~SceneHierarchyPanel() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
	void DrawEntityNode(Cinnamon::Entity entity);
private:
	Cinnamon::STL::USet<Cinnamon::ECS::EntityID> m_SearchDiscaredEntities;

	struct
	{
		uint32_t First;
		uint32_t Second;
	} m_HierarchyTableRowColors;
};