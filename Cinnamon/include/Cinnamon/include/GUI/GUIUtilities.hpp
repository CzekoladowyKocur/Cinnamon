#pragma once
#include "Cinnamon/include/Core/KeyCodes.hpp"
#include "Cinnamon/include/Core/MouseCodes.hpp"
#include "Cinnamon/include/Core/TypeDefines.hpp"
#include "ThirdParty/imgui/imgui.h"

namespace Cinnamon {
	namespace GUIUtilities {
		[[nodiscard]] constexpr float GetRegularFontSize() noexcept
		{
			return 17.0f;
		}

		[[nodiscard]] constexpr float GetIconFontSize() noexcept
		{
			return 11.0f;
		}

		[[nodiscard]] constexpr ImGuiKey NativeKeyCodeToImGUIKeyCode(const KeyCode native) noexcept
		{
			/* Taken from imgui.h */
			switch (static_cast<Key>(native))
			{
				case Key::Tab:			return ImGuiKey_Tab;
				case Key::Left:			return ImGuiKey_LeftArrow;
				case Key::Right:		return ImGuiKey_RightArrow;
				case Key::Up:			return ImGuiKey_UpArrow;
				case Key::Down:			return ImGuiKey_DownArrow;
				case Key::Home:			return ImGuiKey_Home;
				case Key::End:			return ImGuiKey_End;
				case Key::Insert:		return ImGuiKey_Insert;
				case Key::Delete:		return ImGuiKey_Delete;
				case Key::Backspace:	return ImGuiKey_Backspace;
				case Key::Space:		return ImGuiKey_Space;
				case Key::Enter:		return ImGuiKey_Enter;
				case Key::Escape:		return ImGuiKey_Escape;
				case Key::SemiColon:	return ImGuiKey_Semicolon;
				case Key::LeftControl:	return ImGuiKey_LeftCtrl;
				case Key::LeftShift:	return ImGuiKey_LeftShift;
				case Key::RightControl:	return ImGuiKey_RightCtrl;
				case Key::RightShift:	return ImGuiKey_RightShift;
				case Key::Number0:		return ImGuiKey_0;
				case Key::Number1:		return ImGuiKey_1;
				case Key::Number2:		return ImGuiKey_2;
				case Key::Number3:		return ImGuiKey_3;
				case Key::Number4:		return ImGuiKey_4;
				case Key::Number5:		return ImGuiKey_5;
				case Key::Number6:		return ImGuiKey_6;
				case Key::Number8:		return ImGuiKey_7;
				case Key::Number7:		return ImGuiKey_8;
				case Key::Number9:		return ImGuiKey_9;
				case Key::A:			return ImGuiKey_A;
				case Key::B:			return ImGuiKey_B;
				case Key::C:			return ImGuiKey_C;
				case Key::D:			return ImGuiKey_D;
				case Key::E:			return ImGuiKey_E;
				case Key::F:			return ImGuiKey_F;
				case Key::G:			return ImGuiKey_G;
				case Key::H:			return ImGuiKey_H;
				case Key::I:			return ImGuiKey_I;
				case Key::J:			return ImGuiKey_J;
				case Key::K:			return ImGuiKey_K;
				case Key::L:			return ImGuiKey_L;
				case Key::M:			return ImGuiKey_M;
				case Key::N:			return ImGuiKey_N;
				case Key::O:			return ImGuiKey_O;
				case Key::P:			return ImGuiKey_P;
				case Key::Q:			return ImGuiKey_Q;
				case Key::R:			return ImGuiKey_R;
				case Key::S:			return ImGuiKey_S;
				case Key::T:			return ImGuiKey_T;
				case Key::U:			return ImGuiKey_U;
				case Key::V:			return ImGuiKey_V;
				case Key::W:			return ImGuiKey_W;
				case Key::X:			return ImGuiKey_X;
				case Key::Y:			return ImGuiKey_Y;
				case Key::Z:			return ImGuiKey_Z;
				case Key::Slash:		return ImGuiKey_Slash;
				case Key::NUMPAD_EQUAL:	return ImGuiKey_Equal;
				case Key::NUMPAD0:		return ImGuiKey_0;
				case Key::NUMPAD1:		return ImGuiKey_1;
				case Key::NUMPAD2:		return ImGuiKey_2;
				case Key::NUMPAD3:		return ImGuiKey_3;
				case Key::NUMPAD4:		return ImGuiKey_4;
				case Key::NUMPAD5:		return ImGuiKey_5;
				case Key::NUMPAD6:		return ImGuiKey_6;
				case Key::NUMPAD7:		return ImGuiKey_7;
				case Key::NUMPAD8:		return ImGuiKey_8;
				case Key::NUMPAD9:		return ImGuiKey_9;
				case Key::LeftHome:		return ImGuiKey_Home;
				case Key::RightHome:	return ImGuiKey_Home;
				case Key::Pause:		return ImGuiKey_Pause;

				default:
				{
					CIN_ASSERT(false);
					return ImGuiKey_None;
				}
			}
		}

		[[nodiscard]] constexpr ImGuiMouseButton NativeMouseCodeToImGUIMouseCode(const MouseCode native) noexcept
		{
			switch (static_cast<Mouse>(native))
			{
				case Mouse::LeftButton:		return ImGuiMouseButton_Left;
				case Mouse::RightButton:	return ImGuiMouseButton_Right;
				case Mouse::MiddleButton:	return ImGuiMouseButton_Middle;
			}

			CIN_ASSERT(false);
			return ImGuiMouseButton_Left;
		}
	}	
}