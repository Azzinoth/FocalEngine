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
}

FEInput::~FEInput()
{
}

void FEInput::Update()
{
	MouseScrollXOffset = 0.0;
	MouseScrollYOffset = 0.0;
}

void FEInput::AddKeyCallback(void(*Func)(int, int, int, int))
{
	if (Func != nullptr)
		ClientKeyButtonCallbacks.push_back(Func);
}

void FEInput::AddMouseButtonCallback(void(*Func)(int, int, int))
{
	if (Func != nullptr)
		ClientMouseButtonCallbacks.push_back(Func);
}

void FEInput::AddMouseMoveCallback(void(*Func)(double, double))
{
	if (Func != nullptr)
		ClientMouseMoveCallbacks.push_back(Func);
}

void FEInput::MouseButtonCallback(const int Button, const int Action, const int Mods)
{
	for (size_t i = 0; i < INPUT.ClientMouseButtonCallbacks.size(); i++)
	{
		if (INPUT.ClientMouseButtonCallbacks[i] == nullptr)
			continue;

		INPUT.ClientMouseButtonCallbacks[i](Button, Action, Mods);
	}
}

void FEInput::MouseMoveCallback(double Xpos, double Ypos)
{
	for (size_t i = 0; i < INPUT.ClientMouseMoveCallbacks.size(); i++)
	{
		if (INPUT.ClientMouseMoveCallbacks[i] == nullptr)
			continue;

		INPUT.ClientMouseMoveCallbacks[i](Xpos, Ypos);
	}

	INPUT.LastFrameMouseX = INPUT.MouseX;
	INPUT.LastFrameMouseY = INPUT.MouseY;

	INPUT.MouseX = Xpos;
	INPUT.MouseY = Ypos;
}

void FEInput::KeyButtonCallback(const int Key, const int Scancode, const int Action, const int Mods)
{
	for (size_t i = 0; i < INPUT.ClientKeyButtonCallbacks.size(); i++)
	{
		if (INPUT.ClientKeyButtonCallbacks[i] == nullptr)
			continue;
		INPUT.ClientKeyButtonCallbacks[i](Key, Scancode, Action, Mods);
	}

	if (Key == GLFW_KEY_A && Action == GLFW_PRESS)
	{
		INPUT.bAKeyPressed = true;
	}
	else if (Key == GLFW_KEY_A && Action == GLFW_RELEASE)
	{
		INPUT.bAKeyPressed = false;
	}

	if (Key == GLFW_KEY_W && Action == GLFW_PRESS)
	{
		INPUT.bWKeyPressed = true;
	}
	else if (Key == GLFW_KEY_W && Action == GLFW_RELEASE)
	{
		INPUT.bWKeyPressed = false;
	}

	if (Key == GLFW_KEY_S && Action == GLFW_PRESS)
	{
		INPUT.bSKeyPressed = true;
	}
	else if (Key == GLFW_KEY_S && Action == GLFW_RELEASE)
	{
		INPUT.bSKeyPressed = false;
	}

	if (Key == GLFW_KEY_D && Action == GLFW_PRESS)
	{
		INPUT.bDKeyPressed = true;
	}
	else if (Key == GLFW_KEY_D && Action == GLFW_RELEASE)
	{
		INPUT.bDKeyPressed = false;
	}
}

void FEInput::MouseScrollCallback(const double Xoffset, const double Yoffset)
{
	INPUT.MouseScrollXOffset = Xoffset;
	INPUT.MouseScrollYOffset = Yoffset;

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

	INPUT.MouseX = X;
	INPUT.MouseY = Y;
}

double FEInput::GetMouseX()
{
	return MouseX;
}

double FEInput::GetMouseY()
{
	return MouseY;
}