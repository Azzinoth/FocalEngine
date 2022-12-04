#pragma once

#include "../Editor/FEEditorSubWindows/projectWasModifiedPopUp.h"
#include <functional>
using namespace FocalEngine;

#ifdef FE_WIN_32
const COMDLG_FILTERSPEC OBJ_LOAD_FILTER[] =
{
	{ L"Wavefront OBJ files (*.obj)", L"*.obj" }
};

const COMDLG_FILTERSPEC TEXTURE_LOAD_FILTER[] =
{
	{ L"Image files (*.png; *.jpg; *.bmp)", L"*.png;*.jpg;*.bmp" }
};

const COMDLG_FILTERSPEC ALL_IMPORT_LOAD_FILTER[] =
{
	{ L"All files (*.png; *.jpg; *.bmp; *.obj)", L"*.png;*.jpg;*.bmp;*.obj" },
	{ L"Image files (*.png; *.jpg; *.bmp)", L"*.png;*.jpg;*.bmp" },
	{ L"Wavefront OBJ files (*.obj)", L"*.obj" }
};
#endif

class DragAndDropTarget;
class FEEditor
{
public:
	SINGLETON_PUBLIC_PART(FEEditor)

	void InitializeResources();

	double GetLastMouseX() const;
	void SetLastMouseX(double NewValue);
	double GetLastMouseY() const;
	void SetLastMouseY(double NewValue);

	double GetMouseX() const;
	void SetMouseX(double NewValue);
	double GetMouseY() const;
	void SetMouseY(double NewValue);

	std::string GetObjectNameInClipboard();
	void SetObjectNameInClipboard(std::string NewValue);

	void Render();
	bool bLeftMousePressed = false;
	bool bShiftPressed = false;
private:
	SINGLETON_PRIVATE_PART(FEEditor)

	double LastMouseX, LastMouseY;
	double MouseX, MouseY;
	std::string ObjectNameInClipboard;
	static ImGuiWindow* SceneWindow;
	bool bSceneWindowHovered;

	bool bIsCameraInputActive = false;

	FETexture* SculptBrushIcon = nullptr;
	FETexture* LevelBrushIcon = nullptr;
	FETexture* SmoothBrushIcon = nullptr;
	FETexture* MouseCursorIcon = nullptr;
	FETexture* ArrowToGroundIcon = nullptr;
	FETexture* DrawBrushIcon = nullptr;

	void SetCorrectSceneBrowserColor(FEObject* SceneObject) const;
	void PopCorrectSceneBrowserColor(FEObject* SceneObject);
	ImVec4 TerrainColorSceneBrowser = ImVec4(67.0f / 255.0f, 155.0f / 255.0f, 60.0f / 255.0f, 1.0f);
	ImVec4 EntityColorSceneBrowser = ImVec4(141.0f / 255.0f, 141.0f / 255.0f, 233.0f / 255.0f, 1.0f);
	ImVec4 InstancedEntityColorSceneBrowser = ImVec4(80.0f / 255.0f, 72.0f / 255.0f, 1.0f, 1.0f);
	ImVec4 CameraColorSceneBrowser = ImVec4(0.0f, 215.0f / 255.0f, 201.0f / 255.0f, 1.0f);
	ImVec4 LightColorSceneBrowser = ImVec4(243.0f / 255.0f, 230.0f / 255.0f, 31.0f / 255.0f, 1.0f);

	void DrawCorrectSceneBrowserIcon(const FEObject* SceneObject) const;
	FETexture* EntitySceneBrowserIcon = nullptr;
	FETexture* InstancedEntitySceneBrowserIcon = nullptr;
	FETexture* DirectionalLightSceneBrowserIcon = nullptr;
	FETexture* SpotLightSceneBrowserIcon = nullptr;
	FETexture* PointLightSceneBrowserIcon = nullptr;
	FETexture* TerrainSceneBrowserIcon = nullptr;
	FETexture* CameraSceneBrowserIcon = nullptr;

	DragAndDropTarget* SceneWindowTarget = nullptr;

	static void OnCameraUpdate(FEBasicCamera* Camera);
	static void MouseButtonCallback(int Button, int Action, int Mods);
	static void MouseMoveCallback(double Xpos, double Ypos);
	static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
	static void RenderTargetResizeCallback(int NewW, int NewH);
	static void DropCallback(int Count, const char** Paths);

	void DisplaySceneBrowser();
	bool bSceneBrowserVisible = true;
	void DisplayInspector();
	bool bInspectorVisible = true;
	bool bEffectsWindowVisible = true;
	void DisplayEffectsWindow() const;
	bool bLogWindowVisible = true;
	void DisplayLogWindow() const;

	static void CloseWindowCallBack();

	void ShowTransformConfiguration(FEObject* Object, FETransformComponent* Transform) const;
	void ShowTransformConfiguration(std::string Name, FETransformComponent* Transform) const;

	void DisplayMaterialParameter(FEShaderParam* Param) const;
	void DisplayLightProperties(FELight* Light) const;
	void DisplayLightsProperties() const;

	void ShowInFolderItem(FEObject* Object);

	// ************** Terrain Settings **************
	ImGuiButton* ExportHeightMapButton = nullptr;
	ImGuiButton* ImportHeightMapButton = nullptr;
	ImGuiImageButton* SculptBrushButton = nullptr;
	ImGuiImageButton* LevelBrushButton = nullptr;
	ImGuiImageButton* SmoothBrushButton = nullptr;

	ImGuiImageButton* LayerBrushButton = nullptr;

	static bool EntityChangePrefabTargetCallBack(FEObject* Object, void** EntityPointer);
	static bool TerrainChangeMaterialTargetCallBack(FEObject* Object, void** LayerIndex);

	static FEEntity* EntityToModify;
	static void ChangePrefabOfEntityCallBack(std::vector<FEObject*> SelectionsResult);

	DragAndDropTarget* EntityChangePrefabTarget = nullptr;
	std::vector<int> TerrainChangeMaterialIndecies;
	std::vector<DragAndDropTarget*> TerrainChangeLayerMaterialTargets;

	int HoveredTerrainLayerItem = -1;
	void DisplayTerrainSettings(FETerrain* Terrain);

	int TerrainLayerRenameIndex = -1;
	char TerrainLayerRename[1024];
	bool bLastFrameTerrainLayerRenameEditWasVisiable = false;
	// ************** Terrain Settings END **************
	// 
	// ************** Content Browser **************

	FETexture* FolderIcon = nullptr;
	FETexture* ShaderIcon = nullptr;
	FETexture* VFSBackIcon = nullptr;

	FETexture* TextureContentBrowserIcon = nullptr;
	FETexture* MeshContentBrowserIcon = nullptr;
	FETexture* MaterialContentBrowserIcon = nullptr;
	FETexture* GameModelContentBrowserIcon = nullptr;
	FETexture* PrefabContentBrowserIcon = nullptr;

	void DisplayContentBrowser();
	bool bContentBrowserVisible = true;
	void ChooseTexturesForContentBrowserItem(FETexture*& PreviewTexture, FETexture*& SmallAdditionTypeIcon, ImVec2& UV0, ImVec2& UV1, FEObject* Item);
	void DisplayContentBrowserItems();

	float ContentBrowserItemIconSize = 128.0;
	int ContentBrowserItemUnderMouse = -1;
	int ContentBrowserRenameIndex = -1;
	char ContentBrowserRename[1024];
	bool bLastFrameRenameEditWasVisiable = false;

	std::vector<FEObject*> AllResourcesContentBrowser;
	std::vector<FEObject*> FilteredResourcesContentBrowser;
	char FilterForResourcesContentBrowser[512];
	static FEObject* ItemInFocus;
	std::string ObjTypeFilterForResourcesContentBrowser;
	void UpdateFilterForResourcesContentBrowser();
	FETexture* AllContentBrowserIcon = nullptr;
	ImGuiImageButton* FilterAllTypesButton = nullptr;
	ImGuiImageButton* FilterTextureTypeButton = nullptr;
	ImGuiImageButton* FilterMeshTypeButton = nullptr;
	ImGuiImageButton* FilterMaterialTypeButton = nullptr;
	ImGuiImageButton* FilterGameModelTypeButton = nullptr;
	ImGuiImageButton* FilterPrefabTypeButton = nullptr;

	char FilterForSceneEntities[512];
	bool bShouldOpenContextMenuInContentBrowser = false;
	bool bShouldOpenContextMenuInSceneEntities = false;

	// ************** Drag&Drop **************
	struct DirectoryDragAndDropCallbackInfo
	{
		std::string DirectoryPath;
	};
	std::vector <DirectoryDragAndDropCallbackInfo> DirectoryDragAndDropInfo;
	std::vector<DragAndDropTarget*> ContentBrowserDirectoriesTargets;
	DragAndDropTarget* VFSBackButtonTarget = nullptr;
	DirectoryDragAndDropCallbackInfo VFSBackButtoninfo;

	static bool DirectoryDragAndDropCallback(FEObject* Object, void** Directory)
	{
		const DirectoryDragAndDropCallbackInfo* info = reinterpret_cast<DirectoryDragAndDropCallbackInfo*>(Directory);
		if (Object->GetType() == FE_NULL)
		{
			std::string OldPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (OldPath.back() != '/')
				OldPath += "/";

			VIRTUAL_FILE_SYSTEM.MoveDirectory(OldPath + Object->GetName(), info->DirectoryPath);
		}
		else
		{
			VIRTUAL_FILE_SYSTEM.MoveFile(Object, VIRTUAL_FILE_SYSTEM.GetCurrentPath(), info->DirectoryPath);
		}

		return true;
	}
	
	void UpdateDirectoryDragAndDropTargets();

	// ************** Drag&Drop END **************
	
	// ************** Content Browser END **************

	int TextureUnderMouse = -1;
	int MeshUnderMouse = -1;
	std::string ShaderIdUnderMouse;
	int MaterialUnderMouse = -1;
	int GameModelUnderMouse = -1;
	int EntityUnderMouse = -1;

	bool bGameMode = false;
	bool IsInGameMode() const;
	void SetGameMode(bool GameMode);

	void RenderAllSubWindows();
};

#define EDITOR FEEditor::getInstance()