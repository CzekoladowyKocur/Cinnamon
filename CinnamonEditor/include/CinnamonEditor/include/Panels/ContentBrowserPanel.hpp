#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.hpp"
#include "Cinnamon/include/Core/CinSTL.hpp"
#include "Cinnamon/include/Core/Filesystem.hpp"

using AssetDragCallbackFunction = std::function<void(const std::filesystem::path&)>;
using FilePopupCallbackFunction = std::function<void(const std::filesystem::path&, const bool)>;

class ContentBrowserPanel final : public EditorPanelBase
{
private:
	struct FilePopupSelectionCache
	{
		Cinnamon::STL::Filepath Path;
		bool IsDirectory;

		[[nodiscard]] bool Empty() noexcept
		{
			return Path.empty();
		}

		void Clear() noexcept
		{
			Path.clear();
			IsDirectory = false;
		}
	};
	
	NON_COPYABLE(ContentBrowserPanel)
public:
	explicit ContentBrowserPanel(
		Project*& projectContext,
		Cinnamon::Scene*& sceneContext, 
		Cinnamon::Entity& selectionContext) noexcept;

	virtual ~ContentBrowserPanel() noexcept;

	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnGUIRender() override final;
	virtual void OnEvent(const Cinnamon::Event& event) override final;

	constexpr virtual const char* GetPanelName() const override final;
private:
	void ReconstructFileTreeIfNeeded();
	void RescanFileTreeIfNeeded();
	void OpenFilePopup();
private:
	Cinnamon::STL::Filepath m_WorkingDirectory;
	Cinnamon::STL::Unique<Cinnamon::FileWatcher> m_FileWatcher;
	Cinnamon::STL::Unique<class FileTree> m_FileTree;

	AssetDragCallbackFunction m_AssetDragCallbackFunction;
	FilePopupCallbackFunction m_FilePopupCallbackFunction;
	FilePopupSelectionCache m_FilePopupSelectionCache;
	bool m_OpenFilePopup;

	static constexpr const char* s_FilePopupID{ "##filePopup" };
};