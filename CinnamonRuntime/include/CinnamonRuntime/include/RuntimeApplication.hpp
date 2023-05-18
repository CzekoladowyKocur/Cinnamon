#pragma once
#include "Cinnamon/include/Core/Application.hpp"

namespace Cinnamon {
	class Event;
	class ApplicationRenderEvent;
	class Renderer;
}

class RuntimeApplication final : public Cinnamon::Application
{
private:
	NON_COPYABLE(RuntimeApplication)
public:
	explicit RuntimeApplication() noexcept;
	virtual ~RuntimeApplication() noexcept;

	virtual Errr OnUserInitialize() final override;
	virtual void OnUserShutdown() final override;

	virtual void OnEvent(const Cinnamon::Event& event) final override;
private:
	bool OnApplicationRender(const Cinnamon::ApplicationRenderEvent& event);
private:
	Cinnamon::Layer* m_RuntimeLayer;
};