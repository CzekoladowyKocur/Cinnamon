#pragma once
#include "Cinnamon/include/Core/Core.hpp"

struct ProjectSettings final
{
	Cinnamon::STL::Filepath Directory;
	Cinnamon::STL::Filepath Path;
};

class Project final
{
private:
	NON_COPYABLE(Project)
public:
	explicit Project() noexcept;
	explicit Project(const ProjectSettings& settings) noexcept;
	~Project() noexcept;

	void SetProjectDirectory(const Cinnamon::STL::Filepath& path);
	void SetProjectPath(const Cinnamon::STL::Filepath& path);
	void SetStartScenePath(const Cinnamon::STL::Filepath& path);

	[[nodiscard]] const Cinnamon::STL::Filepath& 
		GetProjectDirectory() const;
	
	[[nodiscard]] const Cinnamon::STL::Filepath& 
		GetProjectPath() const;

	[[nodiscard]] const Cinnamon::STL::Filepath&
		GetStartScenePath() const;
private:
	Cinnamon::STL::Filepath m_ProjectDirectory;
	Cinnamon::STL::Filepath m_ProjectPath;
	Cinnamon::STL::Filepath m_StartScenePath;
private:
	friend class ProjectSerializer;
};

class ProjectSerializer final
{
private:
	NON_COPYABLE(ProjectSerializer)
public:
	explicit ProjectSerializer(Project* const project) noexcept;
	~ProjectSerializer() noexcept = default;

	void Serialize(const Cinnamon::STL::Filepath& projectPath) noexcept(false);
	void Deserialize(const Cinnamon::STL::Filepath& projectPath) noexcept(false);
private:
	Project* const m_Project;
};