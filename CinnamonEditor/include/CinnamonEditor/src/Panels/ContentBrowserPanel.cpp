#include "CinnamonEditor/include/Panels/ContentBrowserPanel.hpp"
#include "Cinnamon/include/Core/Filesystem.hpp"
#include "Cinnamon/include/GUI/Icons.hpp"

#include "CinnamonEditor/include/Project.hpp"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;

class FileTree
{
public:
	enum class EFileType
	{
		None,
		Root,
		Directory,
		RegularFile,
	};

	struct FileNode
	{
		const EFileType FileType;
		const STL::String RealPath;
		const STL::String Filename;
		const STL::String IconifiedName;

		const FileNode* const Parent;
		STL::Vector<FileNode> Children;

		inline explicit FileNode(
			const STL::Filepath& parentRelativePath,
			const STL::Filepath& realPath,
			const FileNode* const parent) noexcept
			:
			FileType(parent ? GetFileType(realPath) : EFileType::Root),
			RealPath(realPath.string()),
			Filename(parentRelativePath.string()),
			IconifiedName(IconifyFilename(Filename, FileType)),
			Parent(parent),
			Children{}
		{}

		inline ~FileNode() noexcept
		{
			Children.clear();
		}

		[[nodiscard]] inline FileNode* FindSubdirectory(const STL::Filepath& parentRelativePath)
		{
			for (auto& file : Children)
				if (file.Filename == parentRelativePath)
					return &file;

			return nullptr;
		}

		inline void AddFile(const STL::Filepath& parentRelativePath, const STL::Filepath& realPath)
		{
			Children.emplace_back(FileNode(parentRelativePath, realPath, this));
		}

		[[nodiscard]] inline EFileType GetFileType(const STL::Filepath& filepath) const
		{
			if (std::filesystem::is_directory(filepath))
				return EFileType::Directory;
			else if (std::filesystem::is_regular_file(filepath))
				return EFileType::RegularFile;

			return EFileType::None;
		}

		[[nodiscard]] inline STL::String IconifyFilename(const STL::String& filename, const EFileType fileType)
		{
			switch (fileType)
			{
			case EFileType::Root:
			{
				constexpr char spacedIcon[]{ ICON_FA_FOLDER_OPEN" " };
				STL::String iconifiedName{ spacedIcon };

				STL::Filepath rootPath{ filename };
				if (rootPath.has_filename())
					iconifiedName += rootPath.filename().string();
				else
					iconifiedName += filename;

				return iconifiedName;
			}

			case EFileType::Directory:
			{
				constexpr char spacedIcon[]{ ICON_FA_FOLDER" " };
				STL::String iconifiedName{ spacedIcon };
				iconifiedName += filename;

				return iconifiedName;
			}

			case EFileType::RegularFile:
			{
				const STL::Filepath asFilepath(filename);
				STL::String spacedIcon;

				[[likely]]
				if (asFilepath.has_extension())
				{
					const STL::String extension{ asFilepath.extension().string() };

					if (extension == ".cpp")
						spacedIcon += ICON_FA_SCROLL" ";
					else if (extension == ".png" || extension == ".jpg")
						spacedIcon += ICON_FA_IMAGE" ";
					else if (extension == ".txt")
						spacedIcon += ICON_FA_PAPERCLIP" ";
					else if (extension == ".cinscene")
						spacedIcon += ICON_FA_GLOBE" ";
					else if (extension == ".cinproj")
						spacedIcon += ICON_FA_ARCHIVE" ";
					else if (extension == ".sln")
						spacedIcon += ICON_FA_CODE" ";
					else if (extension == ".vcxproj")
						spacedIcon += ICON_FA_COGS" ";
					else if (extension == ".user")
						spacedIcon += ICON_FA_USER_COG" ";
					else
						spacedIcon += ICON_FA_FILE" ";
				}
				else
					return filename;

				return spacedIcon + filename;
			}

			[[unlikely]]
			default:
				break;
			}

			return "[UNKNOWN-FILE-TYPE] " + filename;
		}

		inline operator const char* () const
		{
			return IconifiedName.c_str();
		}

		const char* ID() const
		{
			return RealPath.c_str();
		}
	};
public:
	inline explicit FileTree(
		const STL::Filepath& rootPath,
		const AssetDragCallbackFunction& assetDragCallback,
		const FilePopupCallbackFunction& filePopupCallback) noexcept
		:
		m_RootNode(rootPath, rootPath, nullptr),
		m_AssetDragCallbackFunction(assetDragCallback),
		m_FilePopupCallbackFunction(filePopupCallback)
	{
		CIN_ASSERT(assetDragCallback)
	}

	inline ~FileTree() noexcept = default;

	inline void AddFile(const STL::Filepath& file)
	{
		AddFileRecursive(std::filesystem::relative(file, m_RootNode.Filename), file, m_RootNode);
	}

	inline void OnGUIRender()
	{
		ImGui::Unindent();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
		OnGUIRenderRecursive(m_RootNode);
		ImGui::PopStyleVar();
	}
private:
	inline void AddFileRecursive(const STL::Filepath& relativePath, const STL::Filepath& realPath, FileNode& currentFolder)
	{
		const STL::String stringFilepath{ relativePath.string() };
#ifdef CIN_PLATFORM_WINDOWS
		const std::size_t separatorPosition{ stringFilepath.find("\\") };
#elif defined CIN_PLATFORM_LINUX
		const std::size_t separatorPosition{ stringFilepath.find('/') };
#endif
		const STL::String nextFolder{ separatorPosition != STL::String::npos ? stringFilepath.substr(0, separatorPosition) : "" };

		if (!nextFolder.empty())
		{
			FileNode* iterator{ currentFolder.FindSubdirectory(nextFolder) };

			[[unlikely]]
			if (!iterator)
			{
				CIN_ERROR("Failed adding file: {}. No parent subdirectory present!", relativePath.string());
				return;
			}

			AddFileRecursive(stringFilepath.substr(separatorPosition + 1U), realPath, *iterator);
		}
		else
		{
			[[unlikely]]
			if (currentFolder.FindSubdirectory(relativePath))
			{
				CIN_ERROR("File {} already added!", realPath.string());
				return;
			}

			currentFolder.AddFile(relativePath, realPath);
		}
	}

	inline void OnGUIRenderRecursive(const FileNode& currentNode)
	{
		switch (currentNode.FileType)
		{
			case FileTree::EFileType::Root:
			{
				constexpr ImGuiTreeNodeFlags rootFlags
				{
					ImGuiTreeNodeFlags_SpanAvailWidth 	|
					ImGuiTreeNodeFlags_OpenOnArrow 		|
					ImGuiTreeNodeFlags_DefaultOpen 		|
					ImGuiTreeNodeFlags_None
				};

				const ImGuiTreeNodeFlags flags{ rootFlags | (currentNode.Children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None) };
				if (ImGui::TreeNodeEx(currentNode.ID(), currentNode.Parent ? flags : flags | ImGuiTreeNodeFlags_DefaultOpen, currentNode))
				{
					for (const FileNode& node : currentNode.Children)
						OnGUIRenderRecursive(node);

					ImGui::TreePop();
				}
				break;
			}

			case FileTree::EFileType::Directory:
			{
				constexpr ImGuiTreeNodeFlags directoryFlags
				{
					ImGuiTreeNodeFlags_SpanAvailWidth 	|
					ImGuiTreeNodeFlags_OpenOnArrow 		|
					ImGuiTreeNodeFlags_None
				};

				const ImGuiTreeNodeFlags flags{ directoryFlags | (currentNode.Children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None) };
				if (ImGui::TreeNodeEx(currentNode.ID(), currentNode.Parent ? flags : flags | ImGuiTreeNodeFlags_DefaultOpen, currentNode))
				{
					if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Right))
						m_FilePopupCallbackFunction(currentNode.RealPath, true);

					for (const FileNode& node : currentNode.Children)
						OnGUIRenderRecursive(node);

					ImGui::TreePop();
				}
				else
				{
					[[unlikely]]
					if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Right))
						m_FilePopupCallbackFunction(currentNode.RealPath, true);
				}
			} break;

			case FileTree::EFileType::RegularFile:
			{
				constexpr ImGuiTreeNodeFlags regularFileFlags
				{
					ImGuiTreeNodeFlags_SpanAvailWidth 	|
					ImGuiTreeNodeFlags_OpenOnArrow 		|
					ImGuiTreeNodeFlags_Leaf 			|
					ImGuiTreeNodeFlags_None
				};

				if (ImGui::TreeNodeEx(currentNode.ID(), regularFileFlags, currentNode))
					ImGui::TreePop();

				[[unlikely]]
				if (ImGui::IsItemHovered() && ImGui::IsItemClicked(ImGuiMouseButton_Right))
					m_FilePopupCallbackFunction(currentNode.RealPath, false);
				else if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter, false))
					CIN_WARN("Enter on: {}", currentNode.Filename);

				[[unlikely]]
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					m_AssetDragCallbackFunction(currentNode.RealPath);
					ImGui::EndDragDropSource();
				}
			} break;

			[[unlikely]]
			default:
			{
				if (ImGui::TreeNodeEx(currentNode.ID(), 0U, currentNode))
					ImGui::TreePop();

				break;
			}
		}
	}
private:
	FileNode m_RootNode;
	AssetDragCallbackFunction m_AssetDragCallbackFunction;
	FilePopupCallbackFunction m_FilePopupCallbackFunction;
};

InternalScope constinit std::atomic<bool> s_FileTreeNeedsRescan{ true };
ContentBrowserPanel::ContentBrowserPanel(
	ProjectContext		projectContext,
	SceneContext		sceneContext, 
	SelectionContext	selectionContext) noexcept
	:
	EditorPanelBase(projectContext, sceneContext, selectionContext),
	m_FileWatcher(nullptr),
	m_FileTree(nullptr),
	m_AssetDragCallbackFunction(nullptr),
	m_FilePopupCallbackFunction(nullptr),
	m_FilePopupSelectionCache(),
	m_OpenFilePopup(false)
{
	m_AssetDragCallbackFunction = [](const STL::Filepath& filepath) noexcept
	{
		/* Only regular files are accepted and are expected to have an extension */
		CIN_ASSERT(filepath.has_filename() && filepath.has_extension());
		const STL::Filepath extension{ filepath.extension() };
		const STL::String filepathString{ filepath.string() };

		if (extension == ".cpp")
		{
			ImGui::SetDragDropPayload("CPPScriptPayload", filepathString.data(), (filepathString.size() + 1U) * sizeof(STL::String::value_type));
			ImGui::Text((ICON_FA_SCROLL " " + filepath.filename().string()).c_str());
		}
		else if (extension == ".png")
		{
			ImGui::SetDragDropPayload("PNGImagePayload", filepathString.data(), (filepathString.size() + 1U) * sizeof(STL::String::value_type));
			ImGui::Text((ICON_FA_IMAGE " " + filepath.filename().string()).c_str());
		}
		else if (extension == ".cinscene")
		{
			ImGui::SetDragDropPayload("ScenePayload", filepathString.data(), (filepathString.size() + 1U) * sizeof(STL::String::value_type));
			ImGui::Text((ICON_FA_GLOBE " " + filepath.filename().string()).c_str());
		}
		else
		{
			ImGui::Text(ICON_FA_BAN);
			CIN_TRACE("Attempted dragging file with unsupported extension: {}", filepath.string());
		}
	};

	m_FilePopupCallbackFunction = [this](const STL::Filepath& path, const bool isDirectory) noexcept
	{
		m_FilePopupSelectionCache.Path = path;
		m_FilePopupSelectionCache.IsDirectory = isDirectory;
		m_OpenFilePopup = true;
	};
}

ContentBrowserPanel::~ContentBrowserPanel() noexcept
{}

void ContentBrowserPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void ContentBrowserPanel::OnGUIRender()
{
	ImGui::Begin(GetPanelName());
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 2.0f, 2.0f });

	if (m_ProjectContext)
	{
		m_WorkingDirectory = m_ProjectContext->GetProjectDirectory();
		OpenFilePopup();
		ReconstructFileTreeIfNeeded();
		RescanFileTreeIfNeeded();

		[[likely]]
		if (m_FileWatcher->IsWatching())
			m_FileTree->OnGUIRender();
		else
			ImGui::Text("Failed to construct file tree");
	}
	else
		ImGui::Text("No project selected");

	ImGui::PopStyleVar();
	ImGui::End();
}

void ContentBrowserPanel::OnEvent(const Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* ContentBrowserPanel::GetPanelName() const
{
	return "Content Browser";
}

void ContentBrowserPanel::ReconstructFileTreeIfNeeded()
{
	FunctionVariable STL::Filepath lastWatchedPath;
	if (m_WorkingDirectory.empty() or not FileExists(m_WorkingDirectory))
	{
		m_WorkingDirectory.clear();
		m_FileWatcher.reset();
		m_FileTree.reset();
	}
	else if (m_WorkingDirectory != lastWatchedPath)
	{
		STL::ErrorCode error;

		m_FileWatcher = STL::MakeUnique<FileWatcher>(m_WorkingDirectory, 
		[](
			const STL::Filepath filepath, 
			const STL::Optional<STL::Filepath> renamedNew, 
			const EFileAction fileAction, 
			const STL::ErrorCode ec
		) noexcept
		{
			if (ec)
				CIN_WARN("File watcher error: {}", ec.message());
			else
			{
				s_FileTreeNeedsRescan = true;
				switch (fileAction)
				{
					case EFileAction::Created:
					{
						CIN_WARN("Created {}", filepath.string());
					} break;

					case EFileAction::Deleted:
					{
						CIN_WARN("Deleted {}", filepath.string());
					} break;

					case EFileAction::Modified:
					{
						CIN_WARN("Modified {}", filepath.string());
					} break;

					case EFileAction::Renamed:
					{
						CIN_WARN("Renamed {} to {}", filepath.string(), renamedNew.value().string());
					} break;

					case EFileAction::Error:
					{
						if(!filepath.empty())
							CIN_WARN("Err {}", filepath.string());
					} break;
				}
			}
		}, false, error);

		m_FileTree = STL::MakeUnique<FileTree>(m_WorkingDirectory, m_AssetDragCallbackFunction, m_FilePopupCallbackFunction);
		s_FileTreeNeedsRescan = true;

		lastWatchedPath = m_WorkingDirectory;
	}
}

void ContentBrowserPanel::RescanFileTreeIfNeeded()
{
	[[unlikely]]
	if (s_FileTreeNeedsRescan)
	{
		m_FileTree = STL::MakeUnique<FileTree>(m_WorkingDirectory, m_AssetDragCallbackFunction, m_FilePopupCallbackFunction);

		try
		{
			for (const auto& file : STL::RecursiveDirectoryIterator(m_WorkingDirectory))
				m_FileTree->AddFile(file);

			s_FileTreeNeedsRescan = false;
		}
		catch ([[maybe_unused]] const std::exception& exception)
		{
			CIN_WARN("Failed scanning current working directory: {}, Error: {}", m_WorkingDirectory.string(), exception.what());
			ImGui::Text("Failed scanning current working directory: %s", m_WorkingDirectory.string().c_str());
			ImGui::End();

			s_FileTreeNeedsRescan = true;
			return;
		}
	}
}

void ContentBrowserPanel::OpenFilePopup()
{
	if (m_OpenFilePopup)
	{
		ImGui::OpenPopup(s_FilePopupID);
		m_OpenFilePopup = false;
	}

	if (ImGui::BeginPopup(s_FilePopupID, ImGuiWindowFlags_NoMove))
	{
		CIN_ASSERT(!m_FilePopupSelectionCache.Empty());

		if (ImGui::Button("Open"))
		{
			[[unlikely]]
			if (!Platform::OpenInExplorer(m_FilePopupSelectionCache.Path.string()))
				CIN_WARN("Failed to open file {} in explorer", m_FilePopupSelectionCache.Path.string());

			m_FilePopupSelectionCache.Clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	else
		m_FilePopupSelectionCache.Clear();

	const ImVec2 cachedCursorPosition{ ImGui::GetCursorPos() };
	const ImVec2 contentRegionAvailableSize{ ImGui::GetContentRegionAvail() };

	ImGui::Dummy({ contentRegionAvailableSize.x, contentRegionAvailableSize.y + ImGui::GetScrollY() });
	if (m_FilePopupSelectionCache.Empty() && ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("contentBrowserPopup");

	if (ImGui::BeginPopup("contentBrowserPopup"))
	{
		if (ImGui::Button("Open in file explorer"))
		{
			if (Platform::OpenInExplorer(m_WorkingDirectory.string()))
				ImGui::CloseCurrentPopup();
			else
				CIN_ERROR("Failed to open {} in file explorer", m_WorkingDirectory.string());
		}

		ImGui::EndPopup();
	}

	ImGui::SetCursorPos(cachedCursorPosition);
}