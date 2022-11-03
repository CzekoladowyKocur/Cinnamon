#include "CinnamonEditor/include/Panels/ContentBrowserPanel.h"
#include "Cinnamon/include/Core/Filesystem.h"
#include "Cinnamon/include/GUI/Icons.h"

#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"

//std::promise<void> promise;
//std::vector<std::string> filesTriggered;
//std::set<std::thread::id> fileWatchThreads;
//std::mutex mutex;
//const uint32_t expected_triggers = 2u;

using namespace Cinnamon;
#define MY_TEST_LINUX 
ContentBrowserPanel::ContentBrowserPanel() noexcept
	:
	#ifdef MY_TEST_LINUX
	m_WorkingDirectory("/home/dxm/Container"),
	#elif defined MY_TEST_WINDOWS
	m_WorkingDirectory("C:\\Users\\marti\\Desktop\\FINALPLIK"),
	#else
	m_WorkingDirectory(),
	#endif
	m_FileWatcher(nullptr)
{
	if(!FileExists(m_WorkingDirectory))
	{
		m_FileWatcher = nullptr;
		return;
	}

	m_WorkingDirectoryIterator = std::move(STL::RecursiveDirectoryIterator(m_WorkingDirectory));
	m_FileWatcher = cinew Cinnamon::FileWatcher(m_WorkingDirectory.string(), { ".txt", ".cpp" }, [](const STL::String file, const Cinnamon::EFileAction action)
		{
			//std::lock_guard<std::mutex> lock(mutex);
			//fileWatchThreads.insert(std::this_thread::get_id());
			//filesTriggered.push_back(file);
			//if (fileWatchThreads.size() == expected_triggers)
			//	promise.set_value();

			switch (action)
			{
				case Cinnamon::EFileAction::Created:
				{
					CIN_INFO("Created file: {}", file);
				} break;

				case Cinnamon::EFileAction::Modified:
				{
					CIN_INFO("Modified file: {}", file);
				} break;

				case Cinnamon::EFileAction::Deleted:
				{
					CIN_INFO("Deleted file: {}", file);
				} break;

				default:
					break;
			}
		});
}

ContentBrowserPanel::~ContentBrowserPanel() noexcept
{
	[[likely]]
	if (m_FileWatcher)
		cindel m_FileWatcher;

	m_FileWatcher = nullptr;
}

void ContentBrowserPanel::OnUpdate(const Timestep timestep)
{
	CIN_UNUSED(timestep);
}

void ContentBrowserPanel::OnGUIRender()
{
	ImGui::Begin(GetPanelName());

	if (Cinnamon::FileExists(m_WorkingDirectory) && Cinnamon::IsDirectory(m_WorkingDirectory))
	{
		if (ImGui::TreeNode(GetWorkingDirectoryAsString().c_str()))
		{
			DrawDirectoryRecursively(m_WorkingDirectory, [this](const Cinnamon::STL::Filepath& filepath) 
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						if (filepath.extension() == ".cpp")
							ImGui::SetDragDropPayload("Script File", filepath.string().data(), (filepath.string().size() + 1) * sizeof(const char));
						
						ImGui::EndDragDropSource();
					}
				});
			ImGui::TreePop();
		}
	}
	else
		ImGui::Text("No project selected");
	
	ImGui::End();
}

void ContentBrowserPanel::OnEvent(const Cinnamon::Event& event)
{
	CIN_UNUSED(event);
}

constexpr const char* ContentBrowserPanel::GetPanelName() const
{
	return "Content Browser Panel";
}

Cinnamon::STL::String ContentBrowserPanel::GetWorkingDirectoryAsString() const
{
	return m_WorkingDirectory.string();
}

void ContentBrowserPanel::DrawDirectoryRecursively(const Cinnamon::STL::Filepath& directory, const AssetCallbackFunction callback)
{
	for (const auto& file : Cinnamon::STL::DirectoryIterator(directory))
	{
		const Cinnamon::STL::Filepath& filepathEntry{ file.path() };
		const bool isDirectory{ file.is_directory() };
		const bool isRegularFile{ file.is_regular_file() };

		const Cinnamon::STL::String fileName{ PopulateFilenameWithIcon(filepathEntry, isRegularFile) };

		if (isDirectory)
		{
			constexpr ImGuiTreeNodeFlags directoryFlags
			{
				ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_OpenOnArrow
			};

			if (ImGui::TreeNodeEx(fileName.c_str(), directoryFlags))
			{
				DrawDirectoryRecursively(filepathEntry, callback);
				ImGui::TreePop();
			}
		}
		else
		{
			constexpr ImGuiTreeNodeFlags regularFileFlags
			{
				ImGuiTreeNodeFlags_SpanAvailWidth	|
				ImGuiTreeNodeFlags_OpenOnArrow		|
				ImGuiTreeNodeFlags_Leaf
			};
			
			if (ImGui::TreeNodeEx(fileName.c_str(), regularFileFlags))
				ImGui::TreePop();

			callback(filepathEntry);
		}
	}
}

Cinnamon::STL::String ContentBrowserPanel::PopulateFilenameWithIcon(const Cinnamon::STL::Filepath& filepath, const bool isRegularFile)
{
	Cinnamon::STL::String filename;
	CIN_UNUSED(filepath);
	[[likely]]
	if (isRegularFile)
	{
		Cinnamon::STL::Filepath fileExtension{ filepath.extension() };

		if (fileExtension == ".cpp")
			filename += ICON_FA_SCROLL" ";
	}
	else
		filename += ICON_FA_FOLDER" ";

	filename += filepath.filename().string();
	return filename;
}