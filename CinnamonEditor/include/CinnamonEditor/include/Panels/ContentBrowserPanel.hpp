#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "Cinnamon/include/Core/CinSTL.hpp"
#include "Cinnamon/include/Core/Filesystem.hpp"

class ContentBrowserPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(ContentBrowserPanel)
public:
	explicit ContentBrowserPanel(Cinnamon::Scene*& sceneContext, Cinnamon::Entity& selectionContext) noexcept;
	virtual ~ContentBrowserPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
	Cinnamon::STL::Filepath m_WorkingDirectory;

	Cinnamon::STL::Unique<Cinnamon::FileWatcher> m_FileWatcher;
	Cinnamon::STL::Unique<class FileTree> m_FileTree;
};