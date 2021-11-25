#pragma once

#include "../Editor/FEEditorSubWindows/projectWasModifiedPopUp.h"
#include <functional>
using namespace FEGizmoManager;
using namespace FocalEngine;

#ifdef FE_WIN_32
const COMDLG_FILTERSPEC meshLoadFilter[] =
{
	{ L"Wavefront OBJ files (*.obj)", L"*.obj" },
};

const COMDLG_FILTERSPEC textureLoadFilter[] =
{
	{ L"PNG files (*.png)", L"*.png" },
};
#endif

class DragAndDropTarget;
class FEEditor
{
public:
	SINGLETON_PUBLIC_PART(FEEditor)

	void initializeResources();

	double getLastMouseX();
	void setLastMouseX(double newValue);
	double getLastMouseY();
	void setLastMouseY(double newValue);

	double getMouseX();
	void setMouseX(double newValue);
	double getMouseY();
	void setMouseY(double newValue);

	std::string getObjectNameInClipboard();
	void setObjectNameInClipboard(std::string newValue);

	void render();
	bool leftMousePressed = false;
	bool shiftPressed = false;
private:
	SINGLETON_PRIVATE_PART(FEEditor)

	double lastMouseX, lastMouseY;
	double mouseX, mouseY;
	std::string objectNameInClipboard = "";
	static ImGuiWindow* sceneWindow;
	bool sceneWindowHovered;

	bool isCameraInputActive = false;

	FETexture* sculptBrushIcon = nullptr;
	FETexture* levelBrushIcon = nullptr;
	FETexture* smoothBrushIcon = nullptr;
	FETexture* mouseCursorIcon = nullptr;
	FETexture* arrowToGroundIcon = nullptr;
	FETexture* drawBrushIcon = nullptr;

	void setCorrectSceneBrowserColor(FEObject* sceneObject);
	void popCorrectSceneBrowserColor(FEObject* sceneObject);
	ImVec4 terrainColorSceneBrowser = ImVec4(67.0f / 255.0f, 155.0f / 255.0f, 60.0f / 255.0f, 1.0f);
	ImVec4 entityColorSceneBrowser = ImVec4(141.0f / 255.0f, 141.0f / 255.0f, 233.0f / 255.0f, 1.0f);
	ImVec4 instancedEntityColorSceneBrowser = ImVec4(80.0f / 255.0f, 72.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	ImVec4 cameraColorSceneBrowser = ImVec4(0.0f, 215.0f / 255.0f, 201.0f / 255.0f, 1.0f);
	ImVec4 lightColorSceneBrowser = ImVec4(243.0f / 255.0f, 230.0f / 255.0f, 31.0f / 255.0f, 1.0f);

	void drawCorrectSceneBrowserIcon(FEObject* sceneObject);
	FETexture* entitySceneBrowserIcon = nullptr;
	FETexture* instancedEntitySceneBrowserIcon = nullptr;
	FETexture* directionalLightSceneBrowserIcon = nullptr;
	FETexture* spotLightSceneBrowserIcon = nullptr;
	FETexture* pointLightSceneBrowserIcon = nullptr;
	FETexture* terrainSceneBrowserIcon = nullptr;
	FETexture* cameraSceneBrowserIcon = nullptr;

	DragAndDropTarget* sceneWindowTarget = nullptr;

	static void onCameraUpdate(FEBasicCamera* camera);
	static void mouseButtonCallback(int button, int action, int mods);
	static void mouseMoveCallback(double xpos, double ypos);
	static void keyButtonCallback(int key, int scancode, int action, int mods);
	static void renderTargetResizeCallback(int newW, int newH);
	static void dropCallback(int count, const char** paths);

	void displaySceneBrowser();
	bool sceneBrowserVisible = true;
	void displayInspector();
	bool inspectorVisible = true;
	bool effectsWindowVisible = true;
	void displayEffectsWindow();
	bool logWindowVisible = true;
	void displayLogWindow();

	static void closeWindowCallBack();

	void showTransformConfiguration(FEObject* object, FETransformComponent* transform);
	void showTransformConfiguration(std::string name, FETransformComponent* transform);

	void displayMaterialParameter(FEShaderParam* param);
	void displayLightProperties(FELight* light);
	void displayLightsProperties();

	// ************** Terrain Settings **************
	ImGuiButton* exportHeightMapButton = nullptr;
	ImGuiButton* importHeightMapButton = nullptr;
	ImGuiImageButton* sculptBrushButton = nullptr;
	ImGuiImageButton* levelBrushButton = nullptr;
	ImGuiImageButton* smoothBrushButton = nullptr;

	ImGuiImageButton* layerBrushButton = nullptr;

	static bool entityChangeGameModelTargetCallBack(FEObject* object, void** entityPointer);
	static bool terrainChangeMaterialTargetCallBack(FEObject* object, void** layerIndex);

	DragAndDropTarget* entityChangeGameModelTarget = nullptr;
	std::vector<int> terrainChangeMaterialIndecies;
	std::vector<DragAndDropTarget*> terrainChangeLayerMaterialTargets;

	int hoveredTerrainLayerItem = -1;
	void displayTerrainSettings(FETerrain* terrain);

	int terrainLayerRenameIndex = -1;
	char terrainLayerRename[1024];
	bool lastFrameTerrainLayerRenameEditWasVisiable = false;
	// ************** Terrain Settings END **************
	// 
	// ************** Content Browser **************

	FETexture* folderIcon = nullptr;
	FETexture* shaderIcon = nullptr;
	FETexture* VFSBackIcon = nullptr;

	FETexture* meshContentBrowserIcon = nullptr;
	FETexture* materialContentBrowserIcon = nullptr;
	FETexture* gameModelContentBrowserIcon = nullptr;

	void displayContentBrowser();
	bool contentBrowserVisible = true;
	void displayContentBrowserItems();

	int contentBrowserItemUnderMouse = -1;
	int contentBrowserRenameIndex = -1;
	char contentBrowserRename[1024];
	bool lastFrameRenameEditWasVisiable = false;

	std::vector<FEObject*> allResourcesContentBrowser;
	std::vector<FEObject*> filteredResourcesContentBrowser;
	char filterForResourcesContentBrowser[512];

	bool isOpenContextMenuInContentBrowser = false;
	bool isOpenContextMenuInSceneEntities = false;
	int activeTabContentBrowser = 0;

	// ************** Drag&Drop **************
	struct directoryDragAndDropCallbackInfo
	{
		std::string directoryPath;
	};
	std::vector <directoryDragAndDropCallbackInfo> directoryDragAndDropInfo;
	std::vector<DragAndDropTarget*> contentBrowserDirectoriesTargets;
	DragAndDropTarget* VFSBackButtonTarget = nullptr;
	directoryDragAndDropCallbackInfo VFSBackButtoninfo;

	static bool directoryDragAndDropCallback(FEObject* object, void** directory)
	{
		directoryDragAndDropCallbackInfo* info = reinterpret_cast<directoryDragAndDropCallbackInfo*>(directory);
		if (object->getType() == FE_NULL)
		{
			std::string oldPath = VIRTUAL_FILE_SYSTEM.getCurrentPath();
			if (oldPath.back() != '/')
				oldPath += "/";

			VIRTUAL_FILE_SYSTEM.moveDirectory(oldPath + object->getName(), info->directoryPath);
		}
		else
		{
			VIRTUAL_FILE_SYSTEM.moveFile(object, VIRTUAL_FILE_SYSTEM.getCurrentPath(), info->directoryPath);
		}

		return true;
	}
	
	void updateDirectoryDragAndDropTargets();

	// ************** Drag&Drop END **************
	
	// ************** Content Browser END **************

	int textureUnderMouse = -1;
	int meshUnderMouse = -1;
	std::string shaderIDUnderMouse = "";
	int materialUnderMouse = -1;
	int gameModelUnderMouse = -1;
	int entityUnderMouse = -1;

	bool gameMode = false;
	bool isInGameMode();
	void setGameMode(bool gameMode);

	void renderAllSubWindows();
};

#define EDITOR FEEditor::getInstance()