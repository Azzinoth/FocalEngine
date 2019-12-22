#pragma once

#include "FECoreIncludes.h"

namespace FocalEngine
{
	class FEInput
	{
	public:
		static FEInput& getInstance()
		{
			if (!_instance)
				_instance = new FEInput();

			return *_instance;
		}
		~FEInput();
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		void(*mouseButtonCallbackImpl)(int, int, int);

		static void keyButtonCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		void(*keyButtonCallbackImpl)(int, int, int, int);
	private:
		static FEInput* _instance;
		FEInput();
		SINGLETON_COPY_ASSIGN_PART(FEInput)
	};
}