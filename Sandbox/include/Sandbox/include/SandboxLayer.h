#pragma once
#include "Cinnamon/include/Core/Layer.h"
#include "Cinnamon/include/Core/Window.h"
#include "Cinnamon/include/Renderer/Renderer.h"

using namespace Cinnamon;

class SandboxLayer final : public Layer
{
public:
	SandboxLayer() noexcept;
	virtual ~SandboxLayer() noexcept;

	virtual void OnAttach() override final;
	virtual void OnUpdate(const Timestep timestep) override final;
	virtual void OnDetach() override final;

	virtual void OnEvent(const Event& event) override final;
private:
	STL::Vector<STL::Unique<Cinnamon::Window>> m_Windows;
	STL::Vector<STL::Unique<Cinnamon::Renderer>> m_Renderers;
};
