#pragma once
#include "Cinnamon/include/Core/Core.h"

class EditorPanelBase
{
private:
	NON_COPYABLE(EditorPanelBase);
public:
	constexpr explicit EditorPanelBase() noexcept = default;
	constexpr virtual ~EditorPanelBase() noexcept = default;
public:
};