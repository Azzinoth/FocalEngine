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

	bool isCameraInputActive = false;

	FETexture* sculptBrushIcon = nullptr;
	FETexture* levelBrushIcon = nullptr;
	FETexture* smoothBrushIcon = nullptr;
	FETexture* mouseCursorIcon = nullptr;
	FETexture* arrowToGroundIcon = nullptr;

	static void onCameraUpdate(FEBasicCamera* camera);
	static void mouseButtonCallback(int button, int action, int mods);
	static void mouseMoveCallback(double xpos, double ypos);
	static void keyButtonCallback(int key, int scancode, int action, int mods);

	void displaySceneEntities();
	void displayContentBrowser();

	void displayShadersContentBrowser();
	void displayMeshesContentBrowser();
	void displayMaterialContentBrowser();
	void displayGameModelContentBrowser();
	void displayTexturesContentBrowser();
	//void displayPostProcessContentBrowser();
	void displayTerrainContentBrowser();
	void displayEffectsContentBrowser();

	static void closeWindowCallBack();

	void showPosition(std::string objectName, glm::vec3& position);
	void showRotation(std::string objectName, glm::vec3& rotation);
	void showScale(std::string objectName, glm::vec3& scale);
	void showTransformConfiguration(std::string objectName, FETransformComponent* transform);

	void displayMaterialPrameter(FEShaderParam* param);
	void displayLightProperties(FELight* light);
	void displayLightsProperties();

	int textureUnderMouse = -1;
	int meshUnderMouse = -1;
	std::string shaderNameUnderMouse = "";
	int materialUnderMouse = -1;
	int gameModelUnderMouse = -1;
	int entityUnderMouse = -1;

	bool isOpenContextMenuInContentBrowser = false;
	int activeTabContentBrowser = 0;
};

#define EDITOR FEEditor::getInstance()