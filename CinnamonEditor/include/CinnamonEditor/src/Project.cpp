#include "CinnamonEditor/include/Project.hpp"

using namespace Cinnamon;
Project::Project() noexcept
	:
	m_ProjectDirectory(),
	m_ProjectPath()
{}

Project::Project(const ProjectSettings& settings) noexcept
	:
	m_ProjectDirectory(settings.Directory),
	m_ProjectPath(settings.Path)
{}

Project::~Project() noexcept
{}

void Project::SetProjectDirectory(const Cinnamon::STL::Filepath & path)
{
	m_ProjectDirectory = path;
}

void Project::SetProjectPath(const Cinnamon::STL::Filepath& path)
{
	m_ProjectPath = path;
}

void Project::SetStartScenePath(const Cinnamon::STL::Filepath& path)
{
	m_StartScenePath = path;
}

const STL::Filepath& Project::GetProjectDirectory() const
{
	return m_ProjectDirectory;
}

const Cinnamon::STL::Filepath& Project::GetProjectPath() const
{
	return m_ProjectPath;
}

const Cinnamon::STL::Filepath& Project::GetStartScenePath() const
{
	return m_StartScenePath;
}

ProjectSerializer::ProjectSerializer(Project* const project) noexcept
	:
	m_Project(project)
{
	CIN_ASSERT(m_Project);
}

void ProjectSerializer::Serialize(const STL::Filepath& projectPath) noexcept(false)
{
	std::ofstream projectFile(projectPath, std::ios::binary);
	if (!projectFile.is_open())
		throw std::runtime_error("Failed to open project file");

	projectFile << m_Project->m_ProjectDirectory;
	projectFile << m_Project->m_ProjectPath;
	projectFile << m_Project->m_StartScenePath;
	projectFile.close();
}

void ProjectSerializer::Deserialize(const STL::Filepath& projectPath) noexcept(false)
{
	if (not std::filesystem::exists(projectPath))
		throw std::runtime_error("Specified project path doesn't exist");

	std::ifstream projectFile(projectPath, std::ios::binary);
	if (!projectFile.is_open())
		throw std::runtime_error("Failed to open project file");

	projectFile >> m_Project->m_ProjectDirectory;
	projectFile >> m_Project->m_ProjectPath;
	projectFile >> m_Project->m_StartScenePath;
	projectFile.close();
}