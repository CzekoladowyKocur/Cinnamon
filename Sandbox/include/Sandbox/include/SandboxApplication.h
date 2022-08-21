#pragma once
#include "Cinnamon/include/Core/Application.h"

class SandboxApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(SandboxApplication)
public:
	explicit SandboxApplication() noexcept;
	constexpr ~SandboxApplication() noexcept = default;

	virtual bool OnUserInitialize() override;
	virtual bool OnUserShutdown() override;
private:
};