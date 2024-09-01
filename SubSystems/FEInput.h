#pragma once

#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEInput
	{
	public:
		SINGLETON_PUBLIC_PART(FEInput)

		void AddKeyCallback(void(*Func)(int, int, int, int));
		void AddMouseButtonCallback(void(*Func)(int, int, int));
		void AddMouseMoveCallback(void(*Func)(double, double));

		void Update();

		double GetMouseX();
		double GetMouseY();

		bool bAKeyPressed = false;
		bool bWKeyPressed = false;
		bool bSKeyPressed = false;
		bool bDKeyPressed = false;
		double LastFrameMouseX = 0.0;
		double LastFrameMouseY = 0.0;
		double MouseScrollXOffset = 0.0;
		double MouseScrollYOffset = 0.0;

		void SetMousePosition(int X, int Y, bool bScreenPosition = true);
	private:
		SINGLETON_PRIVATE_PART(FEInput)

		double MouseX, MouseY;

		static void MouseButtonCallback(int Button, int Action, int Mods);
		std::vector<void(*)(int, int, int)> ClientMouseButtonCallbacks;

		static void MouseMoveCallback(double Xpos, double Ypos);
		std::vector<void(*)(double, double)> ClientMouseMoveCallbacks;

		static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
		std::vector<void(*)(int, int, int, int)> ClientKeyButtonCallbacks;

		static void MouseScrollCallback(double Xoffset, double Yoffset);
		std::vector<void(*)(double, double)> ClientMouseScrollCallbacks;
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetInput();
	#define INPUT (*static_cast<FEInput*>(GetInput()))
#else
	#define INPUT FEInput::GetInstance()
#endif
}