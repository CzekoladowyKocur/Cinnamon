#pragma once
#include "Cinnamon/include/Core/Application.h"

class EditorApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(EditorApplication)
public:
	explicit EditorApplication() noexcept;
	virtual ~EditorApplication() noexcept;

	virtual bool OnUserInitialize() override;
	virtual bool OnUserShutdown() override;
private:
	Cinnamon::Layer* m_EditorLayer;
};