#include "FEInput.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetInput()
{
	return FEInput::GetInstancePointer();
}
#endif

FEInput::FEInput()
{
	APPLICATION.GetMainWindow()->AddOnMouseButtonCallback(&FEInput::MouseButtonCallback);
	APPLICATION.GetMainWindow()->AddOnMouseMoveCallback(&FEInput::MouseMoveCallback);
	APPLICATION.GetMainWindow()->AddOnKeyCallback(&FEInput::KeyButtonCallback);
	APPLICATION.GetMainWindow()->AddOnScrollCallback(&FEInput::MouseScrollCallback);

	KeyMap[FEInputKey::FE_KEY_UNKNOWN] = FEInputKeyInfo(FEInputKey::FE_KEY_UNKNOWN);
	KeyMap[FEInputKey::FE_KEY_SPACE] = FEInputKeyInfo(FEInputKey::FE_KEY_SPACE);
	KeyMap[FEInputKey::FE_KEY_APOSTROPHE] = FEInputKeyInfo(FEInputKey::FE_KEY_APOSTROPHE);
	KeyMap[FEInputKey::FE_KEY_COMMA] = FEInputKeyInfo(FEInputKey::FE_KEY_COMMA);
	KeyMap[FEInputKey::FE_KEY_MINUS] = FEInputKeyInfo(FEInputKey::FE_KEY_MINUS);
	KeyMap[FEInputKey::FE_KEY_PERIOD] = FEInputKeyInfo(FEInputKey::FE_KEY_PERIOD);
	KeyMap[FEInputKey::FE_KEY_SLASH] = FEInputKeyInfo(FEInputKey::FE_KEY_SLASH);
	KeyMap[FEInputKey::FE_KEY_0] = FEInputKeyInfo(FEInputKey::FE_KEY_0);
	KeyMap[FEInputKey::FE_KEY_1] = FEInputKeyInfo(FEInputKey::FE_KEY_1);
	KeyMap[FEInputKey::FE_KEY_2] = FEInputKeyInfo(FEInputKey::FE_KEY_2);
	KeyMap[FEInputKey::FE_KEY_3] = FEInputKeyInfo(FEInputKey::FE_KEY_3);
	KeyMap[FEInputKey::FE_KEY_4] = FEInputKeyInfo(FEInputKey::FE_KEY_4);
	KeyMap[FEInputKey::FE_KEY_5] = FEInputKeyInfo(FEInputKey::FE_KEY_5);
	KeyMap[FEInputKey::FE_KEY_6] = FEInputKeyInfo(FEInputKey::FE_KEY_6);
	KeyMap[FEInputKey::FE_KEY_7] = FEInputKeyInfo(FEInputKey::FE_KEY_7);
	KeyMap[FEInputKey::FE_KEY_8] = FEInputKeyInfo(FEInputKey::FE_KEY_8);
	KeyMap[FEInputKey::FE_KEY_9] = FEInputKeyInfo(FEInputKey::FE_KEY_9);
	KeyMap[FEInputKey::FE_KEY_SEMICOLON] = FEInputKeyInfo(FEInputKey::FE_KEY_SEMICOLON);
	KeyMap[FEInputKey::FE_KEY_EQUAL] = FEInputKeyInfo(FEInputKey::FE_KEY_EQUAL);
	KeyMap[FEInputKey::FE_KEY_A] = FEInputKeyInfo(FEInputKey::FE_KEY_A);
	KeyMap[FEInputKey::FE_KEY_B] = FEInputKeyInfo(FEInputKey::FE_KEY_B);
	KeyMap[FEInputKey::FE_KEY_C] = FEInputKeyInfo(FEInputKey::FE_KEY_C);
	KeyMap[FEInputKey::FE_KEY_D] = FEInputKeyInfo(FEInputKey::FE_KEY_D);
	KeyMap[FEInputKey::FE_KEY_E] = FEInputKeyInfo(FEInputKey::FE_KEY_E);
	KeyMap[FEInputKey::FE_KEY_F] = FEInputKeyInfo(FEInputKey::FE_KEY_F);
	KeyMap[FEInputKey::FE_KEY_G] = FEInputKeyInfo(FEInputKey::FE_KEY_G);
	KeyMap[FEInputKey::FE_KEY_H] = FEInputKeyInfo(FEInputKey::FE_KEY_H);
	KeyMap[FEInputKey::FE_KEY_I] = FEInputKeyInfo(FEInputKey::FE_KEY_I);
	KeyMap[FEInputKey::FE_KEY_J] = FEInputKeyInfo(FEInputKey::FE_KEY_J);
	KeyMap[FEInputKey::FE_KEY_K] = FEInputKeyInfo(FEInputKey::FE_KEY_K);
	KeyMap[FEInputKey::FE_KEY_L] = FEInputKeyInfo(FEInputKey::FE_KEY_L);
	KeyMap[FEInputKey::FE_KEY_M] = FEInputKeyInfo(FEInputKey::FE_KEY_M);
	KeyMap[FEInputKey::FE_KEY_N] = FEInputKeyInfo(FEInputKey::FE_KEY_N);
	KeyMap[FEInputKey::FE_KEY_O] = FEInputKeyInfo(FEInputKey::FE_KEY_O);
	KeyMap[FEInputKey::FE_KEY_P] = FEInputKeyInfo(FEInputKey::FE_KEY_P);
	KeyMap[FEInputKey::FE_KEY_Q] = FEInputKeyInfo(FEInputKey::FE_KEY_Q);
	KeyMap[FEInputKey::FE_KEY_R] = FEInputKeyInfo(FEInputKey::FE_KEY_R);
	KeyMap[FEInputKey::FE_KEY_S] = FEInputKeyInfo(FEInputKey::FE_KEY_S);
	KeyMap[FEInputKey::FE_KEY_T] = FEInputKeyInfo(FEInputKey::FE_KEY_T);
	KeyMap[FEInputKey::FE_KEY_U] = FEInputKeyInfo(FEInputKey::FE_KEY_U);
	KeyMap[FEInputKey::FE_KEY_V] = FEInputKeyInfo(FEInputKey::FE_KEY_V);
	KeyMap[FEInputKey::FE_KEY_W] = FEInputKeyInfo(FEInputKey::FE_KEY_W);
	KeyMap[FEInputKey::FE_KEY_X] = FEInputKeyInfo(FEInputKey::FE_KEY_X);
	KeyMap[FEInputKey::FE_KEY_Y] = FEInputKeyInfo(FEInputKey::FE_KEY_Y);
	KeyMap[FEInputKey::FE_KEY_Z] = FEInputKeyInfo(FEInputKey::FE_KEY_Z);
	KeyMap[FEInputKey::FE_KEY_LEFT_BRACKET] = FEInputKeyInfo(FEInputKey::FE_KEY_LEFT_BRACKET);
	KeyMap[FEInputKey::FE_KEY_BACKSLASH] = FEInputKeyInfo(FEInputKey::FE_KEY_BACKSLASH);
	KeyMap[FEInputKey::FE_KEY_RIGHT_BRACKET] = FEInputKeyInfo(FEInputKey::FE_KEY_RIGHT_BRACKET);
	KeyMap[FEInputKey::FE_KEY_GRAVE_ACCENT] = FEInputKeyInfo(FEInputKey::FE_KEY_GRAVE_ACCENT);
	KeyMap[FEInputKey::FE_KEY_WORLD_1] = FEInputKeyInfo(FEInputKey::FE_KEY_WORLD_1);
	KeyMap[FEInputKey::FE_KEY_WORLD_2] = FEInputKeyInfo(FEInputKey::FE_KEY_WORLD_2);

	KeyMap[FEInputKey::FE_KEY_ESCAPE] = FEInputKeyInfo(FEInputKey::FE_KEY_ESCAPE);
	KeyMap[FEInputKey::FE_KEY_ENTER] = FEInputKeyInfo(FEInputKey::FE_KEY_ENTER);
	KeyMap[FEInputKey::FE_KEY_TAB] = FEInputKeyInfo(FEInputKey::FE_KEY_TAB);
	KeyMap[FEInputKey::FE_KEY_BACKSPACE] = FEInputKeyInfo(FEInputKey::FE_KEY_BACKSPACE);
	KeyMap[FEInputKey::FE_KEY_INSERT] = FEInputKeyInfo(FEInputKey::FE_KEY_INSERT);
	KeyMap[FEInputKey::FE_KEY_DELETE] = FEInputKeyInfo(FEInputKey::FE_KEY_DELETE);
	KeyMap[FEInputKey::FE_KEY_RIGHT] = FEInputKeyInfo(FEInputKey::FE_KEY_RIGHT);
	KeyMap[FEInputKey::FE_KEY_LEFT] = FEInputKeyInfo(FEInputKey::FE_KEY_LEFT);
	KeyMap[FEInputKey::FE_KEY_DOWN] = FEInputKeyInfo(FEInputKey::FE_KEY_DOWN);
	KeyMap[FEInputKey::FE_KEY_UP] = FEInputKeyInfo(FEInputKey::FE_KEY_UP);
	KeyMap[FEInputKey::FE_KEY_PAGE_UP] = FEInputKeyInfo(FEInputKey::FE_KEY_PAGE_UP);
	KeyMap[FEInputKey::FE_KEY_PAGE_DOWN] = FEInputKeyInfo(FEInputKey::FE_KEY_PAGE_DOWN);
	KeyMap[FEInputKey::FE_KEY_HOME] = FEInputKeyInfo(FEInputKey::FE_KEY_HOME);
	KeyMap[FEInputKey::FE_KEY_END] = FEInputKeyInfo(FEInputKey::FE_KEY_END);
	KeyMap[FEInputKey::FE_KEY_CAPS_LOCK] = FEInputKeyInfo(FEInputKey::FE_KEY_CAPS_LOCK);
	KeyMap[FEInputKey::FE_KEY_SCROLL_LOCK] = FEInputKeyInfo(FEInputKey::FE_KEY_SCROLL_LOCK);
	KeyMap[FEInputKey::FE_KEY_NUM_LOCK] = FEInputKeyInfo(FEInputKey::FE_KEY_NUM_LOCK);
	KeyMap[FEInputKey::FE_KEY_PRINT_SCREEN] = FEInputKeyInfo(FEInputKey::FE_KEY_PRINT_SCREEN);
	KeyMap[FEInputKey::FE_KEY_PAUSE] = FEInputKeyInfo(FEInputKey::FE_KEY_PAUSE);
	KeyMap[FEInputKey::FE_KEY_F1] = FEInputKeyInfo(FEInputKey::FE_KEY_F1);
	KeyMap[FEInputKey::FE_KEY_F2] = FEInputKeyInfo(FEInputKey::FE_KEY_F2);
	KeyMap[FEInputKey::FE_KEY_F3] = FEInputKeyInfo(FEInputKey::FE_KEY_F3);
	KeyMap[FEInputKey::FE_KEY_F4] = FEInputKeyInfo(FEInputKey::FE_KEY_F4);
	KeyMap[FEInputKey::FE_KEY_F5] = FEInputKeyInfo(FEInputKey::FE_KEY_F5);
	KeyMap[FEInputKey::FE_KEY_F6] = FEInputKeyInfo(FEInputKey::FE_KEY_F6);
	KeyMap[FEInputKey::FE_KEY_F7] = FEInputKeyInfo(FEInputKey::FE_KEY_F7);
	KeyMap[FEInputKey::FE_KEY_F8] = FEInputKeyInfo(FEInputKey::FE_KEY_F8);
	KeyMap[FEInputKey::FE_KEY_F9] = FEInputKeyInfo(FEInputKey::FE_KEY_F9);
	KeyMap[FEInputKey::FE_KEY_F10] = FEInputKeyInfo(FEInputKey::FE_KEY_F10);
	KeyMap[FEInputKey::FE_KEY_F11] = FEInputKeyInfo(FEInputKey::FE_KEY_F11);
	KeyMap[FEInputKey::FE_KEY_F12] = FEInputKeyInfo(FEInputKey::FE_KEY_F12);
	KeyMap[FEInputKey::FE_KEY_F13] = FEInputKeyInfo(FEInputKey::FE_KEY_F13);
	KeyMap[FEInputKey::FE_KEY_F14] = FEInputKeyInfo(FEInputKey::FE_KEY_F14);
	KeyMap[FEInputKey::FE_KEY_F15] = FEInputKeyInfo(FEInputKey::FE_KEY_F15);
	KeyMap[FEInputKey::FE_KEY_F16] = FEInputKeyInfo(FEInputKey::FE_KEY_F16);
	KeyMap[FEInputKey::FE_KEY_F17] = FEInputKeyInfo(FEInputKey::FE_KEY_F17);
	KeyMap[FEInputKey::FE_KEY_F18] = FEInputKeyInfo(FEInputKey::FE_KEY_F18);
	KeyMap[FEInputKey::FE_KEY_F19] = FEInputKeyInfo(FEInputKey::FE_KEY_F19);
	KeyMap[FEInputKey::FE_KEY_F20] = FEInputKeyInfo(FEInputKey::FE_KEY_F20);
	KeyMap[FEInputKey::FE_KEY_F21] = FEInputKeyInfo(FEInputKey::FE_KEY_F21);
	KeyMap[FEInputKey::FE_KEY_F22] = FEInputKeyInfo(FEInputKey::FE_KEY_F22);
	KeyMap[FEInputKey::FE_KEY_F23] = FEInputKeyInfo(FEInputKey::FE_KEY_F23);
	KeyMap[FEInputKey::FE_KEY_F24] = FEInputKeyInfo(FEInputKey::FE_KEY_F24);
	KeyMap[FEInputKey::FE_KEY_F25] = FEInputKeyInfo(FEInputKey::FE_KEY_F25);
	KeyMap[FEInputKey::FE_KEY_KP_0] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_0);
	KeyMap[FEInputKey::FE_KEY_KP_1] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_1);
	KeyMap[FEInputKey::FE_KEY_KP_2] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_2);
	KeyMap[FEInputKey::FE_KEY_KP_3] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_3);
	KeyMap[FEInputKey::FE_KEY_KP_4] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_4);
	KeyMap[FEInputKey::FE_KEY_KP_5] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_5);
	KeyMap[FEInputKey::FE_KEY_KP_6] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_6);
	KeyMap[FEInputKey::FE_KEY_KP_7] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_7);
	KeyMap[FEInputKey::FE_KEY_KP_8] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_8);
	KeyMap[FEInputKey::FE_KEY_KP_9] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_9);
	KeyMap[FEInputKey::FE_KEY_KP_DECIMAL] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_DECIMAL);
	KeyMap[FEInputKey::FE_KEY_KP_DIVIDE] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_DIVIDE);
	KeyMap[FEInputKey::FE_KEY_KP_MULTIPLY] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_MULTIPLY);
	KeyMap[FEInputKey::FE_KEY_KP_SUBTRACT] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_SUBTRACT);
	KeyMap[FEInputKey::FE_KEY_KP_ADD] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_ADD);
	KeyMap[FEInputKey::FE_KEY_KP_ENTER] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_ENTER);
	KeyMap[FEInputKey::FE_KEY_KP_EQUAL] = FEInputKeyInfo(FEInputKey::FE_KEY_KP_EQUAL);
	KeyMap[FEInputKey::FE_KEY_LEFT_SHIFT] = FEInputKeyInfo(FEInputKey::FE_KEY_LEFT_SHIFT);
	KeyMap[FEInputKey::FE_KEY_LEFT_CONTROL] = FEInputKeyInfo(FEInputKey::FE_KEY_LEFT_CONTROL);
	KeyMap[FEInputKey::FE_KEY_LEFT_ALT] = FEInputKeyInfo(FEInputKey::FE_KEY_LEFT_ALT);
	KeyMap[FEInputKey::FE_KEY_LEFT_SUPER] = FEInputKeyInfo(FEInputKey::FE_KEY_LEFT_SUPER);
	KeyMap[FEInputKey::FE_KEY_RIGHT_SHIFT] = FEInputKeyInfo(FEInputKey::FE_KEY_RIGHT_SHIFT);
	KeyMap[FEInputKey::FE_KEY_RIGHT_CONTROL] = FEInputKeyInfo(FEInputKey::FE_KEY_RIGHT_CONTROL);
	KeyMap[FEInputKey::FE_KEY_RIGHT_ALT] = FEInputKeyInfo(FEInputKey::FE_KEY_RIGHT_ALT);
	KeyMap[FEInputKey::FE_KEY_RIGHT_SUPER] = FEInputKeyInfo(FEInputKey::FE_KEY_RIGHT_SUPER);
	KeyMap[FEInputKey::FE_KEY_MENU] = FEInputKeyInfo(FEInputKey::FE_KEY_MENU);

	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_1].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_2].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_3].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_4].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_5].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_6].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_7].State = FE_RELEASED;
	MouseState.Buttons[FEInputMouseButton::FE_MOUSE_BUTTON_8].State = FE_RELEASED;
}

FEInput::~FEInput()
{
}

void FEInput::Update()
{
	MouseState.ScrollXOffset = 0.0;
	MouseState.ScrollYOffset = 0.0;

	KeyModifier = FEInputKeyModifier::FE_MOD_NONE;
}

std::string FEInput::AddKeyCallback(std::function<void(int, int, int, int)> UserOnKeyButtonCallback)
{
	if (UserOnKeyButtonCallback == nullptr)
	{
		LOG.Add("UserOnKeyButtonCallback is nullptr", "FE_INPUT", FE_LOG_WARNING);
		return "";
	}

	std::pair NewCallback = std::make_pair(UNIQUE_ID.GetUniqueHexID(), UserOnKeyButtonCallback);
	UserOnKeyButtonCallbackFuncs.push_back(NewCallback);

	return NewCallback.first;
}

std::string FEWindow::AddOnDropCallback(std::function<void(int, const char**)> UserOnDropCallback)
{
	std::pair NewCallback = std::make_pair(UNIQUE_ID.GetUniqueHexID(), UserOnDropCallback);
	UserOnDropCallbackFuncs.push_back(NewCallback);

	return NewCallback.first;
}

std::string FEInput::AddMouseButtonCallback(std::function<void(int, int, int)> UserOnMouseButtonCallback)
{
	if (UserOnMouseButtonCallback == nullptr)
	{
		LOG.Add("UserOnMouseButtonCallback is nullptr", "FE_INPUT", FE_LOG_WARNING);
		return "";
	}

	std::pair NewCallback = std::make_pair(UNIQUE_ID.GetUniqueHexID(), UserOnMouseButtonCallback);
	UserOnMouseButtonCallbackFuncs.push_back(NewCallback);

	return NewCallback.first;
}

std::string FEInput::AddMouseMoveCallback(std::function<void(double, double)> UserOnMouseMoveCallback)
{
	if (UserOnMouseMoveCallback == nullptr)
	{
		LOG.Add("UserOnMouseMoveCallback is nullptr", "FE_INPUT", FE_LOG_WARNING);
		return "";
	}

	std::pair NewCallback = std::make_pair(UNIQUE_ID.GetUniqueHexID(), UserOnMouseMoveCallback);
	UserOnMouseMoveCallbackFuncs.push_back(NewCallback);

	return NewCallback.first;
}

void FEInput::MouseButtonCallback(const int Button, const int Action, const int Mods)
{
	FEInputMouseButton TempButton = static_cast<FEInputMouseButton>(Button);
	INPUT.MouseState.Buttons[TempButton].State = static_cast<FEInputKeyState>(Action);

	for (size_t i = 0; i < INPUT.UserOnMouseButtonCallbackFuncs.size(); i++)
	{
		if (INPUT.UserOnMouseButtonCallbackFuncs[i].second == nullptr)
			continue;

		INPUT.UserOnMouseButtonCallbackFuncs[i].second(Button, Action, Mods);
	}
}

void FEInput::MouseMoveCallback(double Xpos, double Ypos)
{
	for (size_t i = 0; i < INPUT.UserOnMouseMoveCallbackFuncs.size(); i++)
	{
		if (INPUT.UserOnMouseMoveCallbackFuncs[i].second == nullptr)
			continue;

		INPUT.UserOnMouseMoveCallbackFuncs[i].second(Xpos, Ypos);
	}

	INPUT.MouseState.X = Xpos;
	INPUT.MouseState.Y = Ypos;
}

void FEInput::KeyButtonCallback(const int Key, const int Scancode, const int Action, const int Mods)
{
	FEInputKey TempKey = static_cast<FEInputKey>(Key);
	INPUT.KeyMap[TempKey].State = static_cast<FEInputKeyState>(Action);
	INPUT.KeyModifier = static_cast<FEInputKeyModifier>(Mods);

	for (size_t i = 0; i < INPUT.UserOnKeyButtonCallbackFuncs.size(); i++)
	{
		if (INPUT.UserOnKeyButtonCallbackFuncs[i].second == nullptr)
			continue;

		INPUT.UserOnKeyButtonCallbackFuncs[i].second(Key, Scancode, Action, Mods);
	}
}

void FEInput::MouseScrollCallback(const double Xoffset, const double Yoffset)
{
	INPUT.MouseState.ScrollXOffset = Xoffset;
	INPUT.MouseState.ScrollYOffset = Yoffset;

	for (size_t i = 0; i < INPUT.ClientMouseScrollCallbacks.size(); i++)
	{
		if (INPUT.ClientMouseScrollCallbacks[i] == nullptr)
			continue;

		INPUT.ClientMouseScrollCallbacks[i](Xoffset, Yoffset);
	}
}

void FEInput::SetMousePosition(int X, int Y, bool bScreenPosition)
{
	if (bScreenPosition)
	{
		glfwSetCursorPos(APPLICATION.GetMainWindow()->GetGlfwWindow(), X - APPLICATION.GetMainWindow()->GetXPosition(), Y - APPLICATION.GetMainWindow()->GetYPosition());
		X -= APPLICATION.GetMainWindow()->GetXPosition();
		Y -= APPLICATION.GetMainWindow()->GetYPosition();
	}
	else
	{
		glfwSetCursorPos(APPLICATION.GetMainWindow()->GetGlfwWindow(), X, Y);
	}

	INPUT.MouseState.X = X;
	INPUT.MouseState.Y = Y;
}

double FEInput::GetMouseX()
{
	return MouseState.X;
}

double FEInput::GetMouseY()
{
	return MouseState.Y;
}

void FEInput::EndFrame()
{
	
}

FEInputKeyInfo FEInput::GetKeyInfo(FEInputKey Key)
{
	if (KeyMap.find(Key) == KeyMap.end())
	{
		LOG.Add("Key with code: " + std::to_string(Key) + " not found in KeyMap", "FE_INPUT", FE_LOG_WARNING);
		return FEInputKeyInfo();
	}
	
	return KeyMap[Key];
}

FEInputKeyModifier FEInput::GetKeyModifier()
{
	return KeyModifier;
}

FEInputMouseState FEInput::GetMouseState()
{
	return MouseState;
}

void FEInput::RemoveCallback(std::string CallbackID)
{
	for (int i = 0; i < UserOnMouseButtonCallbackFuncs.size(); i++)
	{
		if (UserOnMouseButtonCallbackFuncs[i].first == CallbackID)
		{
			UserOnMouseButtonCallbackFuncs.erase(UserOnMouseButtonCallbackFuncs.begin() + i);
			return;
		}
	}

	for (int i = 0; i < UserOnMouseMoveCallbackFuncs.size(); i++)
	{
		if (UserOnMouseMoveCallbackFuncs[i].first == CallbackID)
		{
			UserOnMouseMoveCallbackFuncs.erase(UserOnMouseMoveCallbackFuncs.begin() + i);
			return;
		}
	}

	for (int i = 0; i < UserOnKeyButtonCallbackFuncs.size(); i++)
	{
		if (UserOnKeyButtonCallbackFuncs[i].first == CallbackID)
		{
			UserOnKeyButtonCallbackFuncs.erase(UserOnKeyButtonCallbackFuncs.begin() + i);
			return;
		}
	}
}