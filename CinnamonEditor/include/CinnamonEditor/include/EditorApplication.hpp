#pragma once
#include "Cinnamon/include/Core/Application.hpp"

class EditorApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(EditorApplication)
public:
	explicit EditorApplication() noexcept;
	virtual ~EditorApplication() noexcept;

	virtual Errr OnUserInitialize() override;
	virtual void OnUserShutdown() override;
private:
	Cinnamon::Layer* m_EditorLayer;
};