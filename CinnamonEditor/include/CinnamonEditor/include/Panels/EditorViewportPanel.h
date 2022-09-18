#pragma once
#include "CinnamonEditor/include/Panels/EditorPanelBase.h"

class EditorViewportPanel final : public EditorPanelBase
{
private:
	NON_COPYABLE(EditorViewportPanel);
public:
	constexpr EditorViewportPanel() noexcept = default;
	constexpr virtual ~EditorViewportPanel() noexcept = default;
};