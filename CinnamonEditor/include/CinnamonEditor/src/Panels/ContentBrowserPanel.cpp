#include "CinnamonEditor/include/Panels/ContentBrowserPanel.h"
#include "Cinnamon/include/Core/Filesystem.h"
#include "Cinnamon/include/Core/TypeDefines.h"
#include "Cinnamon/include/GUI/Icons.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

using namespace Cinnamon;

class FileTree
{
public:
	enum class EFileType
	{
		None,
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
			FileType(GetFileType(realPath)),
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
						if (extension == ".png" || extension == ".jpg")
							spacedIcon += ICON_FA_IMAGE" ";
						if (extension == ".txt")
							spacedIcon += ICON_FA_PAPERCLIP" ";
						if (extension == ".cinscene")
							spacedIcon += ICON_FA_GLOBE" ";
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
	inline explicit FileTree(const STL::Filepath& rootPath) noexcept
		:
		m_RootNode(rootPath, rootPath, nullptr)
	{}

	inline ~FileTree() noexcept = default;

	inline void AddFile(const STL::Filepath& file)
	{
		AddFileRecursive(std::filesystem::relative(file, m_RootNode.Filename), file, m_RootNode);
	}

	inline void OnGUIRender()
	{
		OnGUIRenderRecursive(m_RootNode);
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
			case FileTree::EFileType::Directory:
			{
				constexpr ImGuiTreeNodeFlags directoryFlags
				{
					ImGuiTreeNodeFlags_SpanAvailWidth	|
					ImGuiTreeNodeFlags_OpenOnArrow		|
					0U
				};

				const ImGuiTreeNodeFlags flags = directoryFlags | (currentNode.Children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None);
				if (ImGui::TreeNodeEx(currentNode.ID(), currentNode.Parent ? flags : flags | ImGuiTreeNodeFlags_DefaultOpen, currentNode))
				{
					for (const FileNode& node : currentNode.Children)
						OnGUIRenderRecursive(node);

					ImGui::TreePop();
				}
			} break;

			case FileTree::EFileType::RegularFile:
			{
				constexpr ImGuiTreeNodeFlags regularFileFlags
				{
					ImGuiTreeNodeFlags_SpanAvailWidth	|
					ImGuiTreeNodeFlags_OpenOnArrow		|
					ImGuiTreeNodeFlags_Leaf				|
					0U
				};

				[[unlikely]]
				if (ImGui::TreeNodeEx(currentNode.ID(), regularFileFlags, currentNode))
					ImGui::TreePop();
			} break;

			[[unlikely]]
			default:
			{
				[[unlikely]]
				if (ImGui::TreeNodeEx(currentNode.ID(), 0U, currentNode))
					ImGui::TreePop();

				break;
			}
		}
	}
private:
	FileNode m_RootNode;
};

InternalScope constinit std::atomic<bool> s_FileTreeNeedsRescan{ true };
#define MY_TEST_LINUX
ContentBrowserPanel::ContentBrowserPanel() noexcept
	:
	#ifdef MY_TEST_LINUX
	m_WorkingDirectory(""),
	#elif defined MY_TEST_WINDOWS
	m_WorkingDirectory(""),
	#else
	m_WorkingDirectory(),
	#endif
	m_FileWatcher(nullptr),
	m_FileTree(nullptr)
{
	if(!FileExists(m_WorkingDirectory))
	{
		m_FileWatcher.reset();
		m_FileTree.reset();
		return;
	}

	m_FileTree = STL::MakeUnique<FileTree>(m_WorkingDirectory);
	m_FileWatcher = STL::MakeUnique<FileWatcher>(STL::String(m_WorkingDirectory.string()), STL::InitializerList<STL::String>{}, [](const STL::String file, const EFileAction action) noexcept
	{
		CIN_UNUSED(file);
		s_FileTreeNeedsRescan = true;
		switch (action)
		{
			case EFileAction::Created:
			{
				CIN_INFO("Created file: {}", file);
			} break;
			case EFileAction::Modified:
			{
				CIN_INFO("Modified file: {}", file);
			} break;	
			case EFileAction::Deleted:
			{
				CIN_INFO("Deleted file: {}", file);
			} break;
			case EFileAction::RenamedOld:
			{
				CIN_INFO("Renamed file (old): {}", file);
			} break;
			
			case EFileAction::RenamedNew:
			{
				CIN_INFO("Renamed file (new): {}", file);
			} break;

			default:
				break;
		}
	});
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

	[[likely]]
	if (FileExists(m_WorkingDirectory) && IsDirectory(m_WorkingDirectory))
	{
		[[unlikely]]
		if (s_FileTreeNeedsRescan)
		{
			m_FileTree.reset();
			m_FileTree = STL::MakeUnique<FileTree>(m_WorkingDirectory);
			
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

		ImGui::Unindent();
		m_FileTree->OnGUIRender();
	}
	else
		ImGui::Text("No project selected");
	
	ImGui::End();
}

void ContentBrowserPanel::OnEvent(const Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* ContentBrowserPanel::GetPanelName() const
{
	return "Content Browser Panel";
}