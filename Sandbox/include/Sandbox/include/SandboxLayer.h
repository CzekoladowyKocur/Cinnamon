#pragma once
#include "Cinnamon/include/Core/Layer.h"
#include "Cinnamon/include/Core/Window.h"

namespace Cinnamon {
	class Renderer;
}

class SandboxLayer final : public Cinnamon::Layer
{
public:
	SandboxLayer() noexcept;
	virtual ~SandboxLayer() noexcept;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;

	virtual void OnEvent(const Cinnamon::Event& event) override final;
private:
	Cinnamon::STL::Vector<Cinnamon::STL::Unique<Cinnamon::Window>> m_Windows;
	Cinnamon::STL::Vector<Cinnamon::Renderer*> m_Renderers;
};
