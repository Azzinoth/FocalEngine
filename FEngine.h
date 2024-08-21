#pragma once

#include "Renderer\FERenderer.h"
#include "..\SubSystems\FEOpenXR\FEOpenXR.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEngine
	{
		//FIX ME! It should not be here.
		friend class FECameraSystem;
	public:
		SINGLETON_PUBLIC_PART(FEngine)

		void InitWindow(int Width = 1920 * 2, int Height = 1080 * 2, std::string WindowTitle = "FEWindow");

		void BeginFrame(bool InternalCall = false);
		void EndFrame(bool InternalCall = false);

		void Render(bool InternalCall = false);

		bool IsNotTerminated();
		void Terminate();

		void SetWindowCaption(std::string NewCaption);
		void AddWindowResizeCallback(void(*Func)(int, int));
		void AddWindowCloseCallback(void(*Func)());
		void AddKeyCallback(void(*Func)(int, int, int, int));
		void AddMouseButtonCallback(void(*Func)(int, int, int));
		void AddMouseMoveCallback(void(*Func)(double, double));
		void AddDropCallback(void(*Func)(int, const char**));

		void RenderTo(FEFramebuffer* RenderTo);

		double GetCpuTime();
		double GetGpuTime();

		FEPostProcess* CreatePostProcess(std::string Name, int ScreenWidth = -1, int ScreenHeight = -1);
		void SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith);

		FEViewport* GetDefaultViewport();

		bool IsSimplifiedRenderingModeActive();
		void ActivateSimplifiedRenderingMode();

		bool IsVsyncEnabled();
		void SetVsyncEnabled(bool NewValue);

		void DisableVR();
		bool EnableVR();
		bool IsVRInitializedCorrectly();
		bool IsVREnabled();

		void AddOnAfterUpdateCallback(std::function<void()> Callback);

		// FIX ME! Need proper INPUT system.
		double GetMouseX();
		double GetMouseY();

		std::string AddViewport(ImGuiWindow* ImGuiWindowPointer);
		std::string AddViewport(FEWindow* FEWindowPointer);

		void AddOnViewportMovedCallback(std::function<void(std::string)> Callback);
		void AddOnViewportResizeCallback(std::function<void(std::string)> Callback);
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		double CPUTime, GPUTime;
		double CurrentDeltaTime;

		// FIX ME! Need proper INPUT system.
		double MouseX, MouseY;

		bool bSimplifiedRendering = false;
		bool bVsyncEnabled = true;
		bool bVRInitializedCorrectly = false;
		bool bVRActive = false;

		static void WindowResizeCallback(int Width, int Height);
		std::vector<void(*)(int, int)> ClientWindowResizeCallbacks;

		static void MouseButtonCallback(int Button, int Action, int Mods);
		std::vector<void(*)(int, int, int)> ClientMouseButtonCallbacks;

		static void MouseMoveCallback(double Xpos, double Ypos);
		std::vector<void(*)(double, double)> ClientMouseMoveCallbacks;

		static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
		std::vector<void(*)(int, int, int, int)> ClientKeyButtonCallbacks;

		static void DropCallback(int Count, const char** Paths);
		std::vector<void(*)(int, const char**)> ClientDropCallbacks;

		static void MouseScrollCallback(double Xoffset, double Yoffset);
		std::vector<void(*)(double, double)> ClientMouseScrollCallbacks;

		// FIX ME! Need proper INPUT system.
		bool bAKeyPressed = false;
		bool bWKeyPressed = false;
		bool bSKeyPressed = false;
		bool bDKeyPressed = false;
		double LastFrameMouseX = 0.0;
		double LastFrameMouseY = 0.0;
		double MouseScrollXOffset = 0.0;
		double MouseScrollYOffset = 0.0;

		void SetMousePosition(int X, int Y, bool bScreenPosition = true);

		void InternalUpdate();
		std::vector<std::function<void()>> OnAfterUpdateCallbacks;

		std::vector<FEViewport*> Viewports;
		std::vector<std::function<void(std::string)>> OnViewportMovedCallbacks;
		std::vector<std::function<void(std::string)>> OnViewportResizeCallbacks;

		FEViewport* GetViewport(std::string ViewportID);
		void ViewportCheckForModification();
		void ViewportCheckForModificationIndividual(FEViewport* ViewPort, bool& bMoved, bool& bResize);
	};

	#define ENGINE FEngine::GetInstance()

	//extern FOCAL_ENGINE_API FELOG* GetLog();
	extern "C" __declspec(dllexport) void* GetLog();
}