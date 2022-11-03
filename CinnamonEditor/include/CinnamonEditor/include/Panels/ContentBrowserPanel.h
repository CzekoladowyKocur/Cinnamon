#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.h"
#include "Cinnamon/include/Core/CinSTL.h"
#include "Cinnamon/include/Core/Filesystem.h"

class ContentBrowserPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(ContentBrowserPanel)
	using AssetCallbackFunction = std::function<void(const Cinnamon::STL::Filepath&)>;
public:
	explicit ContentBrowserPanel() noexcept;
	virtual ~ContentBrowserPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
	Cinnamon::STL::String GetWorkingDirectoryAsString() const;
	void DrawDirectoryRecursively(const Cinnamon::STL::Filepath& directory, const AssetCallbackFunction callback);
	Cinnamon::STL::String PopulateFilenameWithIcon(const Cinnamon::STL::Filepath& filepath, const bool isRegularFile);
private:
	Cinnamon::STL::Filepath m_WorkingDirectory;
	Cinnamon::STL::RecursiveDirectoryIterator m_WorkingDirectoryIterator;

	Cinnamon::FileWatcher* m_FileWatcher;
	/* TODO: Add file registry */
};