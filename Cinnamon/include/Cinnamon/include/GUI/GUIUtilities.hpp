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
		
		[[nodiscard]] constexpr char NativeKeyCodeToToChar(const KeyCode native) noexcept
		{
			switch (static_cast<Key>(native))
			{
				case Key::A:		return 'A';
				case Key::B:		return 'B';
				case Key::C:		return 'C';
				case Key::D:		return 'D';
				case Key::E:		return 'E';
				case Key::F:		return 'F';
				case Key::G:		return 'G';
				case Key::H:		return 'H';
				case Key::I:		return 'I';
				case Key::J:		return 'J';
				case Key::K:		return 'K';
				case Key::L:		return 'L';
				case Key::M:		return 'M';
				case Key::N:		return 'N';
				case Key::O:		return 'O';
				case Key::P:		return 'P';
				case Key::Q:		return 'Q';
				case Key::R:		return 'R';
				case Key::S:		return 'S';
				case Key::T:		return 'T';
				case Key::U:		return 'U';
				case Key::V:		return 'V';
				case Key::W:		return 'W';
				case Key::X:		return 'X';
				case Key::Y:		return 'Y';
				case Key::Z:		return 'Z';
				case Key::Slash:	return '/';
				case Key::Number0:	return '0';
				case Key::Number1:	return '1';
				case Key::Number2:	return '2';
				case Key::Number3:	return '3';
				case Key::Number4:	return '4';
				case Key::Number5:	return '5';
				case Key::Number6:	return '6';
				case Key::Number8:	return '7';
				case Key::Number7:	return '8';
				case Key::Number9:	return '9';
				case Key::NUMPAD0:	return '0';
				case Key::NUMPAD1:	return '1';
				case Key::NUMPAD2:	return '2';
				case Key::NUMPAD3:	return '3';
				case Key::NUMPAD4:	return '4';
				case Key::NUMPAD5:	return '5';
				case Key::NUMPAD6:	return '6';
				case Key::NUMPAD7:	return '7';
				case Key::NUMPAD8:	return '8';
				case Key::NUMPAD9:	return '9';
				case Key::Space:	return ' ';

				default:			return static_cast<char>(0);
			}
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
				case Key::Shift:		return ImGuiKey_LeftShift;
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
				case Key::F1:			return ImGuiKey_F1;
				case Key::F2:			return ImGuiKey_F2;
				case Key::F3:			return ImGuiKey_F3;
				case Key::F4:			return ImGuiKey_F4;
				case Key::F5:			return ImGuiKey_F5;
				case Key::F6:			return ImGuiKey_F6;
				case Key::F8:			return ImGuiKey_F7;
				case Key::F9:			return ImGuiKey_F8;
				case Key::F10:			return ImGuiKey_F9;
				case Key::F11:			return ImGuiKey_F10;
				case Key::F12:			return ImGuiKey_F11;
				case Key::F13:			return ImGuiKey_F12;
				case Key::LeftAlt:		return ImGuiKey_LeftAlt;
				case Key::Capital:		return ImGuiKey_CapsLock;
				case Key::Control:		return ImGuiKey_LeftCtrl;
				case Key::Comma:		return ImGuiKey_Comma;
				case Key::Period:		return ImGuiKey_Period;

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