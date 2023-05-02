#include "Cinnamon/include/Core/TypeDefines.hpp"

Timestep::Timestep(const Type deltaTime) noexcept
	:
	m_DeltaTime(deltaTime)
{}

Timestep::Type Timestep::GetTimestep() const
{
	return m_DeltaTime;
}

Timestep::operator Type() const
{
	return m_DeltaTime;
}