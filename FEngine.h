#pragma once

#include "Renderer\FERenderer.h"
#include "..\SubSystems\FEOpenXR\FEOpenXR.h"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEngine
	{
	public:
		SINGLETON_PUBLIC_PART(FEngine)

		std::string GetEngineBuildVersion();

		void InitWindow(int Width = 1920 * 2, int Height = 1080 * 2, std::string WindowTitle = "FEWindow");

		void BeginFrame(bool InternalCall = false);
		void EndFrame(bool InternalCall = false);

		void Render(bool InternalCall = false);

		bool IsNotTerminated();
		void Terminate();

		void SetWindowCaption(std::string NewCaption);
		void AddWindowResizeCallback(void(*Func)(int, int));
		void AddWindowCloseCallback(void(*Func)());
		void AddDropCallback(void(*Func)(int, const char**));

		void RenderTo(FEFramebuffer* RenderTo);

		double GetCpuTime();
		double GetGpuTime();

		FEPostProcess* CreatePostProcess(std::string Name, int ScreenWidth = -1, int ScreenHeight = -1);
		void SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith);

		FEViewport* GetDefaultViewport();
		FEViewport* GetViewport(std::string ViewportID);

		bool IsSimplifiedRenderingModeActive();
		void ActivateSimplifiedRenderingMode();

		bool IsVsyncEnabled();
		void SetVsyncEnabled(bool NewValue);

		void DisableVR();
		bool EnableVR();
		bool IsVRInitializedCorrectly();
		bool IsVREnabled();

		void AddOnAfterUpdateCallback(std::function<void()> Callback);

		std::string AddViewport(ImGuiWindow* ImGuiWindowPointer);
		std::string AddViewport(FEWindow* FEWindowPointer);

		void AddOnViewportMovedCallback(std::function<void(std::string)> Callback);
		void AddOnViewportResizeCallback(std::function<void(std::string)> Callback);
	private:
		SINGLETON_PRIVATE_PART(FEngine)

		double CPUTime, GPUTime;
		double CurrentDeltaTime;

		bool bSimplifiedRendering = false;
		bool bVsyncEnabled = true;
		bool bVRInitializedCorrectly = false;
		bool bVRActive = false;

		static void WindowResizeCallback(int Width, int Height);
		std::vector<void(*)(int, int)> ClientWindowResizeCallbacks;

		static void DropCallback(int Count, const char** Paths);
		std::vector<void(*)(int, const char**)> ClientDropCallbacks;

		void InternalUpdate();
		std::vector<std::function<void()>> OnAfterUpdateCallbacks;

		std::vector<FEViewport*> Viewports;
		std::vector<std::function<void(std::string)>> OnViewportMovedCallbacks;
		std::vector<std::function<void(std::string)>> OnViewportResizeCallbacks;

		void ViewportCheckForModification();
		void ViewportCheckForModificationIndividual(FEViewport* ViewPort, bool& bMoved, bool& bResize);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetEngine();
	#define ENGINE (*static_cast<FEngine*>(GetEngine()))
#else
	#define ENGINE FEngine::GetInstance()
#endif
}