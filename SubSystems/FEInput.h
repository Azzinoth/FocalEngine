#pragma once

#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	// Internal representation of GLFW keys
	enum FEInputKey
	{
		FE_KEY_UNKNOWN = -1,
		FE_KEY_SPACE = 32,
		FE_KEY_APOSTROPHE = 39,  /* ' */
		FE_KEY_COMMA = 44,  /* , */
		FE_KEY_MINUS = 45,  /* - */
		FE_KEY_PERIOD = 46,  /* . */
		FE_KEY_SLASH = 47,  /* / */
		FE_KEY_0 = 48,
		FE_KEY_1 = 49,
		FE_KEY_2 = 50,
		FE_KEY_3 = 51,
		FE_KEY_4 = 52,
		FE_KEY_5 = 53,
		FE_KEY_6 = 54,
		FE_KEY_7 = 55,
		FE_KEY_8 = 56,
		FE_KEY_9 = 57,
		FE_KEY_SEMICOLON = 59,  /* ; */
		FE_KEY_EQUAL = 61,  /* = */
		FE_KEY_A = 65,
		FE_KEY_B = 66,
		FE_KEY_C = 67,
		FE_KEY_D = 68,
		FE_KEY_E = 69,
		FE_KEY_F = 70,
		FE_KEY_G = 71,
		FE_KEY_H = 72,
		FE_KEY_I = 73,
		FE_KEY_J = 74,
		FE_KEY_K = 75,
		FE_KEY_L = 76,
		FE_KEY_M = 77,
		FE_KEY_N = 78,
		FE_KEY_O = 79,
		FE_KEY_P = 80,
		FE_KEY_Q = 81,
		FE_KEY_R = 82,
		FE_KEY_S = 83,
		FE_KEY_T = 84,
		FE_KEY_U = 85,
		FE_KEY_V = 86,
		FE_KEY_W = 87,
		FE_KEY_X = 88,
		FE_KEY_Y = 89,
		FE_KEY_Z = 90,
		FE_KEY_LEFT_BRACKET = 91,  /* [ */
		FE_KEY_BACKSLASH = 92,  /* \ */
		FE_KEY_RIGHT_BRACKET = 93,  /* ] */
		FE_KEY_GRAVE_ACCENT = 96,  /* ` */
		FE_KEY_WORLD_1 = 161, /* non-US #1 */
		FE_KEY_WORLD_2 = 162, /* non-US #2 */

		/* Function keys */
		FE_KEY_ESCAPE = 256,
		FE_KEY_ENTER = 257,
		FE_KEY_TAB = 258,
		FE_KEY_BACKSPACE = 259,
		FE_KEY_INSERT = 260,
		FE_KEY_DELETE = 261,
		FE_KEY_RIGHT = 262,
		FE_KEY_LEFT = 263,
		FE_KEY_DOWN = 264,
		FE_KEY_UP = 265,
		FE_KEY_PAGE_UP = 266,
		FE_KEY_PAGE_DOWN = 267,
		FE_KEY_HOME = 268,
		FE_KEY_END = 269,
		FE_KEY_CAPS_LOCK = 280,
		FE_KEY_SCROLL_LOCK = 281,
		FE_KEY_NUM_LOCK = 282,
		FE_KEY_PRINT_SCREEN = 283,
		FE_KEY_PAUSE = 284,
		FE_KEY_F1 = 290,
		FE_KEY_F2 = 291,
		FE_KEY_F3 = 292,
		FE_KEY_F4 = 293,
		FE_KEY_F5 = 294,
		FE_KEY_F6 = 295,
		FE_KEY_F7 = 296,
		FE_KEY_F8 = 297,
		FE_KEY_F9 = 298,
		FE_KEY_F10 = 299,
		FE_KEY_F11 = 300,
		FE_KEY_F12 = 301,
		FE_KEY_F13 = 302,
		FE_KEY_F14 = 303,
		FE_KEY_F15 = 304,
		FE_KEY_F16 = 305,
		FE_KEY_F17 = 306,
		FE_KEY_F18 = 307,
		FE_KEY_F19 = 308,
		FE_KEY_F20 = 309,
		FE_KEY_F21 = 310,
		FE_KEY_F22 = 311,
		FE_KEY_F23 = 312,
		FE_KEY_F24 = 313,
		FE_KEY_F25 = 314,
		FE_KEY_KP_0 = 320,
		FE_KEY_KP_1 = 321,
		FE_KEY_KP_2 = 322,
		FE_KEY_KP_3 = 323,
		FE_KEY_KP_4 = 324,
		FE_KEY_KP_5 = 325,
		FE_KEY_KP_6 = 326,
		FE_KEY_KP_7 = 327,
		FE_KEY_KP_8 = 328,
		FE_KEY_KP_9 = 329,
		FE_KEY_KP_DECIMAL = 330,
		FE_KEY_KP_DIVIDE = 331,
		FE_KEY_KP_MULTIPLY = 332,
		FE_KEY_KP_SUBTRACT = 333,
		FE_KEY_KP_ADD = 334,
		FE_KEY_KP_ENTER = 335,
		FE_KEY_KP_EQUAL = 336,
		FE_KEY_LEFT_SHIFT = 340,
		FE_KEY_LEFT_CONTROL = 341,
		FE_KEY_LEFT_ALT = 342,
		FE_KEY_LEFT_SUPER = 343,
		FE_KEY_RIGHT_SHIFT = 344,
		FE_KEY_RIGHT_CONTROL = 345,
		FE_KEY_RIGHT_ALT = 346,
		FE_KEY_RIGHT_SUPER = 347,
		FE_KEY_MENU = 348
	};

	enum FEInputKeyState
	{
		FE_RELEASED = 0,
		FE_PRESSED = 1,
		FE_REPEAT = 2
	};

	enum FEInputKeyModifier
	{
		FE_MOD_NONE = 0x0000,
		FE_MOD_SHIFT = 0x0001,
		FE_MOD_CONTROL = 0x0002,
		FE_MOD_ALT = 0x0004,
		FE_MOD_SUPER = 0x0008,
		FE_MOD_CAPS_LOCK = 0x0010,
		FE_MOD_NUM_LOCK = 0x0020
	};

	struct FEInputKeyInfo
	{
		FEInputKey Key;
		FEInputKeyState State;
		bool bLastFrameUpdated = false;
			
		FEInputKeyInfo()
		{
			this->Key = FE_KEY_UNKNOWN;
			this->State = FE_RELEASED;
		}

		FEInputKeyInfo(FEInputKey Key, FEInputKeyState State = FE_RELEASED)
		{
			this->Key = Key;
			this->State = State;
		}
	};

	enum FEInputMouseButton
	{
		FE_MOUSE_BUTTON_1 = 0,
		FE_MOUSE_BUTTON_2 = 1,
		FE_MOUSE_BUTTON_3 = 2,
		FE_MOUSE_BUTTON_4 = 3,
		FE_MOUSE_BUTTON_5 = 4,
		FE_MOUSE_BUTTON_6 = 5,
		FE_MOUSE_BUTTON_7 = 6,
		FE_MOUSE_BUTTON_8 = 7,
		FE_MOUSE_BUTTON_LAST = FE_MOUSE_BUTTON_8,
		FE_MOUSE_BUTTON_LEFT = FE_MOUSE_BUTTON_1,
		FE_MOUSE_BUTTON_RIGHT = FE_MOUSE_BUTTON_2,
		FE_MOUSE_BUTTON_MIDDLE = FE_MOUSE_BUTTON_3
	};

	struct FEInputMouseButtonState
	{
		FEInputMouseButton Button;
		FEInputKeyState State;
	};

	struct FEInputMouseState
	{
		double X, Y;
		double ScrollXOffset = 0.0, ScrollYOffset = 0.0;

		std::unordered_map<FEInputMouseButton, FEInputMouseButtonState> Buttons;
	};

	class FOCAL_ENGINE_API FEInput
	{
		friend class FEngine;
	public:
		SINGLETON_PUBLIC_PART(FEInput)

		std::string AddKeyCallback(std::function<void(int, int, int, int)> UserOnKeyButtonCallback);
		std::string AddMouseButtonCallback(std::function<void(int, int, int)> UserOnMouseButtonCallback);
		std::string AddMouseMoveCallback(std::function<void(double, double)> UserOnMouseMoveCallback);

		// TO-DO: It is terrible to obligate the user to remove the callback or application will crash. We should remove it automatically.
		void RemoveCallback(std::string CallbackID);

		void Update();

		FEInputKeyInfo GetKeyInfo(FEInputKey Key);
		FEInputKeyModifier GetKeyModifier();

		FEInputMouseState GetMouseState();

		double GetMouseX();
		double GetMouseY();

		void SetMousePosition(int X, int Y, bool bScreenPosition = true);
	private:
		SINGLETON_PRIVATE_PART(FEInput)

		std::unordered_map<FEInputKey, FEInputKeyInfo> KeyMap;
		FEInputKeyModifier KeyModifier = FE_MOD_NONE;

		FEInputMouseState MouseState;

		static void MouseButtonCallback(int Button, int Action, int Mods);
		std::vector<std::pair<std::string, std::function<void(int, int, int)>>> UserOnMouseButtonCallbackFuncs;

		static void MouseMoveCallback(double Xpos, double Ypos);
		std::vector<std::pair<std::string, std::function<void(double, double)>>> UserOnMouseMoveCallbackFuncs;

		static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
		std::vector<std::pair<std::string, std::function<void(int, int, int, int)>>> UserOnKeyButtonCallbackFuncs;

		static void MouseScrollCallback(double Xoffset, double Yoffset);
		std::vector<void(*)(double, double)> ClientMouseScrollCallbacks;

		void EndFrame();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetInput();
	#define INPUT (*static_cast<FEInput*>(GetInput()))
#else
	#define INPUT FEInput::GetInstance()
#endif
}