#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "CinnamonEditor/include/Popups/ModalPopup.hpp"
#include "Cinnamon/include/Scene/Entity.hpp"

namespace Cinnamon {
	class AssetManager;
	class Texture2D;
}

class EntityPropertiesPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(EntityPropertiesPanel)
public:
	explicit EntityPropertiesPanel(
		Project*& projectContext,
		Cinnamon::Scene*& sceneContext, 
		Cinnamon::Entity& selectionContext,
		const Cinnamon::STL::Unique<Cinnamon::AssetManager>& assetManager) noexcept;
	
	virtual ~EntityPropertiesPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
	void DrawEntityComponents(Cinnamon::Entity entity);
private:
	const Cinnamon::STL::Unique<Cinnamon::AssetManager>& m_AssetManager;
	Cinnamon::STL::Unique<ModalPopup> m_ModalPopup;
	Cinnamon::Texture2D* m_EmptyTexture;
};