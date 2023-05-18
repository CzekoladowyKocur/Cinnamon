#pragma once
#include "Cinnamon/include/Core/Application.hpp"

namespace Cinnamon {
	class Event;
	class ApplicationRenderEvent;
	class Renderer;
}

class SandboxApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(SandboxApplication)
public:
	explicit SandboxApplication() noexcept;
	virtual ~SandboxApplication() noexcept;

	virtual Errr OnUserInitialize() final override;
	virtual void OnUserShutdown() final override;

	virtual void OnEvent(const Cinnamon::Event& event) final override;
private:
	bool OnApplicationRender(const Cinnamon::ApplicationRenderEvent& event);
private:
	Cinnamon::STL::Unique<Cinnamon::Renderer> m_Renderer;
	Cinnamon::Layer* m_SandboxLayer;
};