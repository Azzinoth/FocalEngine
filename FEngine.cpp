#include "FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetEngine()
{
	return FEngine::GetInstancePointer();
}
#endif

FEngine::FEngine()
{
}

FEngine::~FEngine()
{
}

// Included in .cpp rather than .h to prevent engine version macros from being
// exposed to script DLLs built in header-only mode (ENGINE_HEADERS_ONLY).
// Script DLLs capture their own build version at compile time, allowing the
// engine to detect version mismatches when loading them at runtime.
#include "Core/VersionInfo/FOCAL_ENGINE_Version.h"
#include "Core/VersionInfo/FEVersionInfo.h"
FE_DEFINE_VERSION_INFO(FOCAL_ENGINE_)

std::string FEngine::GetEngineVersion()
{
	return GetFOCAL_ENGINE_VersionInfo().GetVersion();
}

int FEngine::GetEngineBuildNumber()
{
	return GetFOCAL_ENGINE_VersionInfo().BuildNumber;
}

std::string FEngine::GetEngineBuildTimestamp()
{
	return GetFOCAL_ENGINE_VersionInfo().BuildTimestamp;
}

std::string FEngine::GetEngineBuildInfo()
{
	return GetFOCAL_ENGINE_VersionInfo().GetBuildInfo();
}

std::string FEngine::GetFullVersion()
{
	return "Focal Engine " + GetFOCAL_ENGINE_VersionInfo().GetFullVersionString();/* GetEngineVersion() + " " + GetEngineBuildInfo();*/
}

bool FEngine::IsNotTerminated()
{
	return APPLICATION.IsNotTerminated();
}

void FEngine::InternalUpdate()
{
	CurrentDeltaTime = CPUTime + GPUTime;

	ViewportCheckForModification();
	
	INSTANCED_RENDERING_SYSTEM.Update();
	SCENE_MANAGER.Update();
	CAMERA_SYSTEM.Update(CurrentDeltaTime);
	VIRTUAL_UI_SYSTEM.Update();

	NATIVE_SCRIPT_SYSTEM.Update(CurrentDeltaTime);

	for (size_t i = 0; i < OnAfterUpdateCallbacks.size(); i++)
	{
		if (OnAfterUpdateCallbacks[i] == nullptr)
			continue;

		OnAfterUpdateCallbacks[i]();
	}

	// Instead of updating TRANSFORM_SYSTEM in the beginning of the frame, we update it here.
	// To ensure that all the other systems are updated before the TRANSFORM_SYSTEM will kick in.
	TRANSFORM_SYSTEM.Update();

	INPUT.Update();
}

void FEngine::BeginFrame(const bool bInternalCall)
{
	if (!APPLICATION.IsNotTerminated())
		return;

	if (!bInternalCall)
		TIME.BeginTimeStamp();

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	RENDERER.BeginFrameDebugLines();

	APPLICATION.BeginFrame();
	if (APPLICATION.GetMainWindow() == nullptr)
		return;

	APPLICATION.GetMainWindow()->BeginFrame();

#ifdef FE_DEBUG_ENABLED
	std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetShaderIDList();
	const std::vector<std::string> TempList = RESOURCE_MANAGER.GetEnginePrivateShaderIDList();
	for (size_t i = 0; i < TempList.size(); i++)
	{
		ShaderList.push_back(TempList[i]);
	}

	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		if (RESOURCE_MANAGER.GetShader(ShaderList[i])->IsDebugRequest())
		{
			RESOURCE_MANAGER.GetShader(ShaderList[i])->ThisFrameDebugBind = 0;
		}
	}
#endif

	InternalUpdate();
}

void FEngine::Render(const bool bInternalCall)
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::Renderable);
	for (size_t i = 0; i < ActiveScenes.size(); i++)
	{
		RENDERER.Render(ActiveScenes[i]);
	}

	if (bVRActive)
		OpenXR_MANAGER.Update();

	APPLICATION.GetMainWindow()->Render();

	if (!bInternalCall)
		CPUTime = TIME.EndTimeStamp();
}

void FEngine::EndFrame(const bool bInternalCall)
{
	RENDERER.EndFrameDebugLines();

	INPUT.EndFrame();

	if (!bInternalCall)
		TIME.BeginTimeStamp();
	APPLICATION.GetMainWindow()->EndFrame();
	APPLICATION.EndFrame();
	if (!bInternalCall)
		GPUTime = TIME.EndTimeStamp();

	// FE_FIX_ME: Since AssetPackage doesn't extract assets directly to memory, we need to delete the directory after the frame completes.
	if (RESOURCE_MANAGER.PrivateEngineAssetPackage != nullptr)
	{
		FILE_SYSTEM.DeleteDirectory(FILE_SYSTEM.GetCurrentWorkingPath() + "/SubSystems");
		delete RESOURCE_MANAGER.PrivateEngineAssetPackage;
		RESOURCE_MANAGER.PrivateEngineAssetPackage = nullptr;
	}

	CurentFrameIndex++;
}

void FEngine::InitWindow(const int Width, const int Height, std::string WindowTitle)
{
	FEWindow* NewWindow = APPLICATION.AddWindow(Width, Height, WindowTitle);
	// Early initialization of INPUT system.
	INPUT;
	APPLICATION.GetMainWindow()->AddOnResizeCallback(&FEngine::WindowResizeCallback);
	APPLICATION.GetMainWindow()->AddOnDropCallback(&FEngine::DropCallback);
	CreateViewport(NewWindow);

	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));

	// tessellation parameter
	FE_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 4));

	RENDERER.Init();

	// Early initialization of the systems.
	TRANSFORM_SYSTEM;
	LIGHT_SYSTEM;
	CAMERA_SYSTEM;
	POINT_CLOUD_SYSTEM;
	INSTANCED_RENDERING_SYSTEM;
	TERRAIN_SYSTEM;
	SKY_DOME_SYSTEM;
	PREFAB_INSTANCE_SYSTEM;
	VIRTUAL_UI_SYSTEM;
	NATIVE_SCRIPT_SYSTEM;
}

void FEngine::SetWindowCaption(const std::string NewCaption)
{
	if (APPLICATION.GetMainWindow() != nullptr)
		APPLICATION.GetMainWindow()->SetTitle(NewCaption);
}

void FEngine::AddWindowResizeCallback(void(*Func)(int, int))
{
	if (Func != nullptr)
		ClientWindowResizeCallbacks.push_back(Func);
}

void FEngine::AddWindowCloseCallback(void(*Func)())
{
	APPLICATION.GetMainWindow()->AddOnCloseCallback(Func);
}

void FEngine::WindowResizeCallback(const int Width, const int Height)
{
	for (size_t i = 0; i < ENGINE.ClientWindowResizeCallbacks.size(); i++)
	{
		if (ENGINE.ClientWindowResizeCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientWindowResizeCallbacks[i](Width, Height);
	}
}

void FEngine::RenderTo(FEFramebuffer* RenderTo)
{
	RenderTo->Bind();
	BeginFrame(true);
	Render(true);
	RenderTo->UnBind();
}

double FEngine::GetCpuTime()
{
	return CPUTime;
}

double FEngine::GetGpuTime()
{
	return GPUTime;
}

FEPostProcess* FEngine::CreatePostProcess(const std::string Name, int ScreenWidth, int ScreenHeight)
{
	if (ScreenWidth < 2 || ScreenHeight < 2)
	{
		ScreenWidth = ENGINE.GetDefaultViewport()->GetWidth();
		ScreenHeight = ENGINE.GetDefaultViewport()->GetHeight();
	}

	return RESOURCE_MANAGER.CreatePostProcess(ScreenWidth, ScreenHeight, Name);
}

void FEngine::Terminate()
{
	APPLICATION.Close();
}

void FEngine::SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith)
{
	RENDERER.SaveScreenshot(FileName, SceneToWorkWith);
}

void FEngine::DropCallback(const int Count, const char** Paths)
{
	for (size_t i = 0; i < ENGINE.ClientDropCallbacks.size(); i++)
	{
		if (ENGINE.ClientDropCallbacks[i] == nullptr)
			continue;

		ENGINE.ClientDropCallbacks[i](Count, Paths);
	}
}

void FEngine::AddDropCallback(void(*Func)(int, const char**))
{
	if (Func != nullptr)
		ClientDropCallbacks.push_back(Func);
}

bool FEngine::IsVsyncEnabled()
{
	return bVsyncEnabled;
}

void FEngine::SetVsyncEnabled(bool NewValue)
{
	bVsyncEnabled = NewValue;
	if (bVsyncEnabled)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}
}

void FEngine::DisableVR()
{
	bVRActive = false;
}

bool FEngine::EnableVR(FERenderingPipeline VRRenderingPipeline)
{
	this->VRRenderingPipeline = VRRenderingPipeline;

	if (!bVRInitializedCorrectly)
		bVRInitializedCorrectly = OpenXR_MANAGER.Init();
	
	bVRActive = bVRInitializedCorrectly;

	return bVRActive;
}

FERenderingPipeline FEngine::GetVRRenderingPipeline() const
{
	return VRRenderingPipeline;
}

std::string FEngine::GetVRApplicationVisibleName() const
{ 
	return VRApplicationVisibleName;
}

void FEngine::SetVRApplicationVisibleName(const std::string& NewName)
{ 
	if (NewName.empty())
	{
		LOG.Add("VR Application Visible Name cannot be empty.", "FE_LOG_OPENXR");
		return;
	}

	VRApplicationVisibleName = NewName; 
}

bool FEngine::IsVRInitializedCorrectly()
{
	return bVRInitializedCorrectly;
}

bool FEngine::IsVREnabled()
{
	return bVRActive;
}

void FEngine::AddOnAfterUpdateCallback(std::function<void()> Callback)
{
	OnAfterUpdateCallbacks.push_back(Callback);
}

std::string FEngine::CreateViewport(ImGuiWindow* ImGuiWindowPointer)
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		if (Viewports[i]->WindowHandle == ImGuiWindowPointer)
			return Viewports[i]->ID;
	}

	FEViewport* NewViewport = new FEViewport();
	NewViewport->Type = FE_VIEWPORT_IMGUI_WINDOW;
	NewViewport->WindowHandle = ImGuiWindowPointer;
	
	Viewports.push_back(NewViewport);

	return NewViewport->ID;
}

std::string FEngine::CreateViewport(FEWindow* FEWindowPointer)
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		if (Viewports[i]->WindowHandle == FEWindowPointer)
			return Viewports[i]->ID;
	}

	FEViewport* NewViewport = new FEViewport();
	NewViewport->Type = FE_VIEWPORT_FEWINDOW;
	NewViewport->WindowHandle = FEWindowPointer;

	Viewports.push_back(NewViewport);

	return NewViewport->ID;
}

FEViewport* FEngine::GetViewport(std::string ViewportID)
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		if (Viewports[i]->ID == ViewportID)
			return Viewports[i];
	}

	return nullptr;
}

void FEngine::ViewportCheckForModificationIndividual(FEViewport* ViewPort, bool& bMoved, bool& bResize)
{
	bMoved = false;
	bResize = false;

	switch (ViewPort->Type)
	{
		case FE_VIEWPORT_VIRTUAL:
		 return;

		case FE_VIEWPORT_OS_WINDOW:
			return;

		case FE_VIEWPORT_GLFW_WINDOW:
			return;
		
		case FE_VIEWPORT_FEWINDOW:
		{
			FEWindow* Window = static_cast<FEWindow*>(ViewPort->WindowHandle);

			if (ViewPort->X != 0 || ViewPort->Y != 0)
				bMoved = true;

			ViewPort->X = 0;
			ViewPort->Y = 0;

			if (ViewPort->Width != Window->GetWidth() || ViewPort->Height != Window->GetHeight())
				bResize = true;

			ViewPort->Width = Window->GetWidth();
			ViewPort->Height = Window->GetHeight();

			return;
		}

		case FE_VIEWPORT_IMGUI_WINDOW:
		{
			ImGuiWindow* Window = static_cast<ImGuiWindow*>(ViewPort->WindowHandle);

			if (ViewPort->X != static_cast<int>(Window->ContentRegionRect.GetTL().x) || ViewPort->Y != static_cast<int>(Window->ContentRegionRect.GetTL().y))
				bMoved = true;

			ViewPort->X = static_cast<int>(Window->ContentRegionRect.GetTL().x);
			ViewPort->Y = static_cast<int>(Window->ContentRegionRect.GetTL().y);

			if (ViewPort->Width != static_cast<int>(Window->ContentRegionRect.GetWidth()) || ViewPort->Height != static_cast<int>(Window->ContentRegionRect.GetHeight()))
				bResize = true;

			ViewPort->Width = static_cast<int>(Window->ContentRegionRect.GetWidth());
			ViewPort->Height = static_cast<int>(Window->ContentRegionRect.GetHeight());

			return;
		}
	}
}

void FEngine::AddOnViewportMovedCallback(std::function<void(std::string)> Callback)
{
	OnViewportMovedCallbacks.push_back(Callback);
}

void FEngine::ViewportCheckForModification()
{
	for (size_t i = 0; i < Viewports.size(); i++)
	{
		bool bMoved, bResize;
		ViewportCheckForModificationIndividual(Viewports[i], bMoved, bResize);

		if (bMoved)
		{
			for (size_t j = 0; j < OnViewportMovedCallbacks.size(); j++)
			{
				if (OnViewportMovedCallbacks[j] != nullptr)
					OnViewportMovedCallbacks[j](Viewports[i]->ID);
			}
		}

		if (bResize)
		{
			for (size_t j = 0; j < OnViewportResizeCallbacks.size(); j++)
			{
				if (OnViewportResizeCallbacks[j] != nullptr)
					OnViewportResizeCallbacks[j](Viewports[i]->ID);
			}
		}
	}
}

void FEngine::AddOnViewportResizeCallback(std::function<void(std::string)> Callback)
{
	OnViewportResizeCallbacks.push_back(Callback);
}

FEViewport* FEngine::GetDefaultViewport()
{
	if (Viewports.size() == 0)
		return nullptr;

	return Viewports[0];
}

unsigned long long FEngine::GetCurrentFrameIndex()
{
	return CurentFrameIndex;
}