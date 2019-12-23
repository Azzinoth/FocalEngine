#pragma once

#include "Renderer\FERenderer.h"
#include "SubSystems\FEInput.h"

namespace FocalEngine
{
	class FEngine
	{
	public:
		static FEngine& getInstance()
		{
			if (!_instance)
				_instance = new FEngine();

			return *_instance;
		}
		~FEngine();

		void createWindow(int width = 1280, int height = 720, std::string WindowTitle = "FEWindow");

		void beginFrame();
		void endFrame();

		bool isWindowOpened();

		void setCamera(FEBasicCamera* newCamera);

		void setKeyCallback(void(*func)(int, int, int, int));
		void setMouseButtonCallback(void(*func)(int, int, int));
		void setMouseMoveCallback(void(*func)(double, double));
	private:
		static FEngine* _instance;

		FEngine();
		SINGLETON_COPY_ASSIGN_PART(FEngine)

		int windowW;
		int windowH;
		std::string windowTitle;

		GLFWwindow* window;
		static void mouseButtonCallback(int button, int action, int mods);
		void(*clientMouseButtonCallbackImpl)(int, int, int) = nullptr;

		static void mouseMoveCallback(double xpos, double ypos);
		void(*clientMouseMoveCallbackImpl)(double, double) = nullptr;

		static void keyButtonCallback(int key, int scancode, int action, int mods);
		void(*clientKeyButtonCallbackImpl)(int, int, int, int) = nullptr;

		FEBasicCamera* currentCamera = nullptr;
	};
}