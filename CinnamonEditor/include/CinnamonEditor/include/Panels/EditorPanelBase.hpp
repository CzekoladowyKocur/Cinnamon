#pragma once
#include "Cinnamon/include/Core/Core.hpp"

namespace Cinnamon {
	class Event;
}

class EditorPanelBase
{
private:
	NON_COPYABLE(EditorPanelBase)
public:
	constexpr explicit EditorPanelBase() noexcept = default;
	constexpr virtual ~EditorPanelBase() noexcept = default;

	virtual void OnUpdate(const Timestep timestep) = 0;
	virtual void OnGUIRender() = 0;
	virtual void OnEvent(const Cinnamon::Event& event) = 0;

	constexpr virtual const char* GetPanelName() const = 0;
public:
};