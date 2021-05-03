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
	//static void windowResizeCallback(int newW, int newH);
	static void renderTargetResizeCallback(int newW, int newH);

	void displaySceneBrowser();
	bool sceneBrowserVisible = true;
	void displayInspector();
	bool inspectorVisible = true;
	bool effectsWindowVisible = true;
	void displayEffectsWindow();
	bool logWindowVisible = true;
	void displayLogWindow();

	static void closeWindowCallBack();

	void showPosition(std::string objectName, glm::vec3& position);
	void showRotation(std::string objectName, glm::vec3& rotation);
	void showScale(std::string objectName, glm::vec3& scale);
	void showTransformConfiguration(FEObject* object, FETransformComponent* transform);
	void showTransformConfiguration(std::string name, FETransformComponent* transform);

	void displayMaterialPrameter(FEShaderParam* param);
	void displayLightProperties(FELight* light);
	void displayLightsProperties();

	// ************** Content Browser **************

	FETexture* folderIcon = nullptr;
	FETexture* shaderIcon = nullptr;
	FETexture* VFSBackIcon = nullptr;

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
};

#define EDITOR FEEditor::getInstance()