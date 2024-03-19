#pragma once

#include <ostream>

namespace Kanto
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	enum class KeyState
	{
		None = -1,
		Pressed,
		Held,
		Released
	};

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	typedef enum class MouseButton : uint16_t
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Left = Button0,
		Right = Button1,
		Middle = Button2
	} Button;


	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, MouseButton button)
	{
		os << static_cast<int32_t>(button);
		return os;
	}
}

// From glfw3.h
#define KN_KEY_SPACE           ::Kanto::Key::Space
#define KN_KEY_APOSTROPHE      ::Kanto::Key::Apostrophe    /* ' */
#define KN_KEY_COMMA           ::Kanto::Key::Comma         /* , */
#define KN_KEY_MINUS           ::Kanto::Key::Minus         /* - */
#define KN_KEY_PERIOD          ::Kanto::Key::Period        /* . */
#define KN_KEY_SLASH           ::Kanto::Key::Slash         /* / */
#define KN_KEY_0               ::Kanto::Key::D0
#define KN_KEY_1               ::Kanto::Key::D1
#define KN_KEY_2               ::Kanto::Key::D2
#define KN_KEY_3               ::Kanto::Key::D3
#define KN_KEY_4               ::Kanto::Key::D4
#define KN_KEY_5               ::Kanto::Key::D5
#define KN_KEY_6               ::Kanto::Key::D6
#define KN_KEY_7               ::Kanto::Key::D7
#define KN_KEY_8               ::Kanto::Key::D8
#define KN_KEY_9               ::Kanto::Key::D9
#define KN_KEY_SEMICOLON       ::Kanto::Key::Semicolon     /* ; */
#define KN_KEY_EQUAL           ::Kanto::Key::Equal         /* = */
#define KN_KEY_A               ::Kanto::Key::A
#define KN_KEY_B               ::Kanto::Key::B
#define KN_KEY_C               ::Kanto::Key::C
#define KN_KEY_D               ::Kanto::Key::D
#define KN_KEY_E               ::Kanto::Key::E
#define KN_KEY_F               ::Kanto::Key::F
#define KN_KEY_G               ::Kanto::Key::G
#define KN_KEY_H               ::Kanto::Key::H
#define KN_KEY_I               ::Kanto::Key::I
#define KN_KEY_J               ::Kanto::Key::J
#define KN_KEY_K               ::Kanto::Key::K
#define KN_KEY_L               ::Kanto::Key::L
#define KN_KEY_M               ::Kanto::Key::M
#define KN_KEY_N               ::Kanto::Key::N
#define KN_KEY_O               ::Kanto::Key::O
#define KN_KEY_P               ::Kanto::Key::P
#define KN_KEY_Q               ::Kanto::Key::Q
#define KN_KEY_R               ::Kanto::Key::R
#define KN_KEY_S               ::Kanto::Key::S
#define KN_KEY_T               ::Kanto::Key::T
#define KN_KEY_U               ::Kanto::Key::U
#define KN_KEY_V               ::Kanto::Key::V
#define KN_KEY_W               ::Kanto::Key::W
#define KN_KEY_X               ::Kanto::Key::X
#define KN_KEY_Y               ::Kanto::Key::Y
#define KN_KEY_Z               ::Kanto::Key::Z
#define KN_KEY_LEFT_BRACKET    ::Kanto::Key::LeftBracket   /* [ */
#define KN_KEY_BACKSLASH       ::Kanto::Key::Backslash     /* \ */
#define KN_KEY_RIGHT_BRACKET   ::Kanto::Key::RightBracket  /* ] */
#define KN_KEY_GRAVE_ACCENT    ::Kanto::Key::GraveAccent   /* ` */
#define KN_KEY_WORLD_1         ::Kanto::Key::World1        /* non-US #1 */
#define KN_KEY_WORLD_2         ::Kanto::Key::World2        /* non-US #2 */

/* Function keys */
#define KN_KEY_ESCAPE          ::Kanto::Key::Escape
#define KN_KEY_ENTER           ::Kanto::Key::Enter
#define KN_KEY_TAB             ::Kanto::Key::Tab
#define KN_KEY_BACKSPACE       ::Kanto::Key::Backspace
#define KN_KEY_INSERT          ::Kanto::Key::Insert
#define KN_KEY_DELETE          ::Kanto::Key::Delete
#define KN_KEY_RIGHT           ::Kanto::Key::Right
#define KN_KEY_LEFT            ::Kanto::Key::Left
#define KN_KEY_DOWN            ::Kanto::Key::Down
#define KN_KEY_UP              ::Kanto::Key::Up
#define KN_KEY_PAGE_UP         ::Kanto::Key::PageUp
#define KN_KEY_PAGE_DOWN       ::Kanto::Key::PageDown
#define KN_KEY_HOME            ::Kanto::Key::Home
#define KN_KEY_END             ::Kanto::Key::End
#define KN_KEY_CAPS_LOCK       ::Kanto::Key::CapsLock
#define KN_KEY_SCROLL_LOCK     ::Kanto::Key::ScrollLock
#define KN_KEY_NUM_LOCK        ::Kanto::Key::NumLock
#define KN_KEY_PRINT_SCREEN    ::Kanto::Key::PrintScreen
#define KN_KEY_PAUSE           ::Kanto::Key::Pause
#define KN_KEY_F1              ::Kanto::Key::F1
#define KN_KEY_F2              ::Kanto::Key::F2
#define KN_KEY_F3              ::Kanto::Key::F3
#define KN_KEY_F4              ::Kanto::Key::F4
#define KN_KEY_F5              ::Kanto::Key::F5
#define KN_KEY_F6              ::Kanto::Key::F6
#define KN_KEY_F7              ::Kanto::Key::F7
#define KN_KEY_F8              ::Kanto::Key::F8
#define KN_KEY_F9              ::Kanto::Key::F9
#define KN_KEY_F10             ::Kanto::Key::F10
#define KN_KEY_F11             ::Kanto::Key::F11
#define KN_KEY_F12             ::Kanto::Key::F12
#define KN_KEY_F13             ::Kanto::Key::F13
#define KN_KEY_F14             ::Kanto::Key::F14
#define KN_KEY_F15             ::Kanto::Key::F15
#define KN_KEY_F16             ::Kanto::Key::F16
#define KN_KEY_F17             ::Kanto::Key::F17
#define KN_KEY_F18             ::Kanto::Key::F18
#define KN_KEY_F19             ::Kanto::Key::F19
#define KN_KEY_F20             ::Kanto::Key::F20
#define KN_KEY_F21             ::Kanto::Key::F21
#define KN_KEY_F22             ::Kanto::Key::F22
#define KN_KEY_F23             ::Kanto::Key::F23
#define KN_KEY_F24             ::Kanto::Key::F24
#define KN_KEY_F25             ::Kanto::Key::F25

/* Keypad */
#define KN_KEY_KP_0            ::Kanto::Key::KP0
#define KN_KEY_KP_1            ::Kanto::Key::KP1
#define KN_KEY_KP_2            ::Kanto::Key::KP2
#define KN_KEY_KP_3            ::Kanto::Key::KP3
#define KN_KEY_KP_4            ::Kanto::Key::KP4
#define KN_KEY_KP_5            ::Kanto::Key::KP5
#define KN_KEY_KP_6            ::Kanto::Key::KP6
#define KN_KEY_KP_7            ::Kanto::Key::KP7
#define KN_KEY_KP_8            ::Kanto::Key::KP8
#define KN_KEY_KP_9            ::Kanto::Key::KP9
#define KN_KEY_KP_DECIMAL      ::Kanto::Key::KPDecimal
#define KN_KEY_KP_DIVIDE       ::Kanto::Key::KPDivide
#define KN_KEY_KP_MULTIPLY     ::Kanto::Key::KPMultiply
#define KN_KEY_KP_SUBTRACT     ::Kanto::Key::KPSubtract
#define KN_KEY_KP_ADD          ::Kanto::Key::KPAdd
#define KN_KEY_KP_ENTER        ::Kanto::Key::KPEnter
#define KN_KEY_KP_EQUAL        ::Kanto::Key::KPEqual

#define KN_KEY_LEFT_SHIFT      ::Kanto::Key::LeftShift
#define KN_KEY_LEFT_CONTROL    ::Kanto::Key::LeftControl
#define KN_KEY_LEFT_ALT        ::Kanto::Key::LeftAlt
#define KN_KEY_LEFT_SUPER      ::Kanto::Key::LeftSuper
#define KN_KEY_RIGHT_SHIFT     ::Kanto::Key::RightShift
#define KN_KEY_RIGHT_CONTROL   ::Kanto::Key::RightControl
#define KN_KEY_RIGHT_ALT       ::Kanto::Key::RightAlt
#define KN_KEY_RIGHT_SUPER     ::Kanto::Key::RightSuper
#define KN_KEY_MENU            ::Kanto::Key::Menu

// Mouse
#define KN_MOUSE_BUTTON_LEFT    ::Kanto::Button::Left
#define KN_MOUSE_BUTTON_RIGHT   ::Kanto::Button::Right
#define KN_MOUSE_BUTTON_MIDDLE  ::Kanto::Button::Middle
