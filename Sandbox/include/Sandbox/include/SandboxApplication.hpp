#pragma once
#include "Cinnamon/include/Core/Application.hpp"

class SandboxApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(SandboxApplication)
public:
	explicit SandboxApplication() noexcept;
	virtual ~SandboxApplication() noexcept;

	virtual Errr OnUserInitialize() override;
	virtual void OnUserShutdown() override;
private:
	Cinnamon::Layer* m_SandboxLayer;
};