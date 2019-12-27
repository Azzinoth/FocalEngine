#pragma once

#include "FECoreIncludes.h"

namespace FocalEngine
{
	class FEInput
	{
	public:
		SINGLETON_PUBLIC_PART(FEInput)

		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		void(*mouseButtonCallbackImpl)(int, int, int);

		static void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
		void(*mouseMoveCallbackImpl)(double, double);

		static void keyButtonCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void(*keyButtonCallbackImpl)(int, int, int, int);
	private:
		SINGLETON_PRIVATE_PART(FEInput)
	};
}