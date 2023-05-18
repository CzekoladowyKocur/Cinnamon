#pragma once
#include "Cinnamon/include/Core/Core.hpp"
#include "Cinnamon/include/Core/CinSTL.hpp"

namespace Cinnamon {
	CIN_FORCE_INLINE bool FileExists(const STL::Filepath& path) noexcept
	{
		return std::filesystem::exists(path);
	}

	CIN_FORCE_INLINE bool IsDirectory(const STL::Filepath& path) noexcept
	{
		CIN_ASSERT(FileExists(path));
		return std::filesystem::is_directory(path);
	}

	CIN_FORCE_INLINE bool IsRegularFile(const STL::Filepath& path) noexcept
	{
		CIN_ASSERT(FileExists(path));
		return std::filesystem::is_regular_file(path);
	}

	CIN_FORCE_INLINE STL::LastFileWriteStamp FileLastWrittenTo(const STL::Filepath& path) noexcept
	{
		CIN_ASSERT(FileExists(path));
		return std::filesystem::last_write_time(path);
	}

	enum class EFileWatcherError
	{
		Unknown = 0,
		InvalidFile,
		SpecifiedFileDoesntExist,
		RegularFileHasNoParentDirectory,
		InternalStateCreationFailed,
		WatchedDirectoryWasDeleted,
		FailedWatchingSubdirectory,
	};

	class FileWatcherErrorCategory final : public std::error_category
	{
	private:
		constexpr FileWatcherErrorCategory(const FileWatcherErrorCategory&) noexcept = delete;
		constexpr FileWatcherErrorCategory(FileWatcherErrorCategory&&) noexcept = delete;
		constexpr FileWatcherErrorCategory& operator=(const FileWatcherErrorCategory&) noexcept = delete;
		constexpr FileWatcherErrorCategory& operator=(FileWatcherErrorCategory&&) noexcept = delete;
	public:
		FileWatcherErrorCategory() noexcept = default;
		constexpr const char* name() const noexcept override final
		{
			return "File Watcher Category";
		}

		constexpr std::string message(const int errorCode) const noexcept override final
		{
			switch (static_cast<EFileWatcherError>(errorCode))
			{
			case EFileWatcherError::InvalidFile: 						return "Specified file is invalid";
			case EFileWatcherError::SpecifiedFileDoesntExist: 			return "Specified file doesn't exist";
			case EFileWatcherError::RegularFileHasNoParentDirectory: 	return "Specified file is regular but has no parent directory";
			case EFileWatcherError::InternalStateCreationFailed: 		return "Internal state creation failed";
			case EFileWatcherError::WatchedDirectoryWasDeleted:			return "Watched directory was deleted, moved or unmounted. If the specified target was a regular file, the parent directory is invalid";
			case EFileWatcherError::FailedWatchingSubdirectory:			return "Failed to watch a subdirectory";
			[[unlikely]] default:
				CIN_ASSERT(false);
				break;
			}

			CIN_ASSERT(false);
			return "Unknown";
			}
		} const fileWatcherErrorCategory;

	inline const FileWatcherErrorCategory& FileWatcherCategory() noexcept
	{
		return fileWatcherErrorCategory;
	}

	/**
	 * Enum class representing all the possible file actions
	 */
	enum class EFileAction
	{
		Error,
		Created,
		Deleted,
		Modified,
		Renamed,
	};

	/**
	 * Converts an EFileAction enum value to it's string representation.
	 * @param fileAction the file action to stringify.
	 */
	constexpr const char* FileActionToString(const EFileAction fileAction) noexcept
	{
		switch (fileAction)
		{
		case EFileAction::Error:		return "Error";
		case EFileAction::Created:		return "Created";
		case EFileAction::Deleted:		return "Deleted";
		case EFileAction::Modified:		return "Modified";
		case EFileAction::Renamed:		return "Renamed";

			[[unlikely]]
		default:
			CIN_ASSERT(false);
			break;
		};

		CIN_ASSERT(false);
		return "UNKNOWN";
	}

	/**
	 * @param Full path to file (new value if renamed).
	 * @param Full path to file if it was renamed (old value), else is left out.
	 * @param Type of file action that had occurred. EFileAction::Error is returned if an error had occurred.
	 * @param Nonzero populated error code if an error had occurred.
	 */
	using FileWatcherCallback = std::function<void(STL::Filepath, STL::Optional<STL::Filepath>, EFileAction, STL::ErrorCode)>;

	/**
	 * File watcher class. Can be used to monitor either an existing directory recursively or a specific file.
	 * If the file doesn't exist, the watcher will listen for it's creation based on it's path.
	 */
	class FileWatcher
	{
	public:
		constexpr FileWatcher(const FileWatcher&) = delete;
		constexpr FileWatcher& operator=(const FileWatcher&) = delete;

		/**
		 * File Watcher constructor.
		 * @param observedPath - Path to observed target. Can be either a filepath or directory path.
		 * @param callback - Callback function.
		 * @param returnAbsolutePath - If true, returns target concatenated directory to absolute path.
		 * @param error - error code, populated on failure.
		 */
		explicit FileWatcher(const STL::Filepath& observedPath, FileWatcherCallback&& callback, const bool returnAbsolutePath, STL::ErrorCode& error) noexcept;

		/**
		 * File Watcher constructor.
		 * @param observedPath - Path to observed target. Can be either a filepath or directory path.
		 * @param callback - Callback function.
		 * @param returnAbsolutePath - If true, returns target concatenated directory to absolute path.
		 * @param error - error code, populated on failure.
		 */
		explicit FileWatcher(const STL::Filepath& observedPath, const FileWatcherCallback& callback, const bool returnAbsolutePath, STL::ErrorCode& error) noexcept;

		/**
		 * File watcher destructor.
		 */
		~FileWatcher() noexcept;

		/**
		 * Returns true if the file watcher is actively monitoring the target.
		 */
		[[nodiscard]] bool IsWatching() const noexcept;
	private:
		void SetupWatcher(const bool useAsbolutePath, STL::ErrorCode& error) noexcept;
		void WatcherThreadWork() const noexcept;

		[[nodiscard]] std::filesystem::path ConstructReturnPath(struct FilewatcherCharacterType* fileNameBuffer, const size_t) const noexcept;
	private:
		mutable std::atomic<bool> m_IsWatching;		// true if actively watching.
		STL::Filepath m_ObservedPath;				// path of observed directory (parent path if observing a file).
		STL::Filepath m_ObservedFile; 				// empty if observing a directory.
		FileWatcherCallback m_Callback;

		std::thread m_WatcherThread;				// watching is performed on a separate blocking thread.
		STL::Unique<struct FileWatcherInternalState> m_InternalState;
	private:
		constexpr static inline size_t s_WatchBufferSize{ 8192U };
	};
}