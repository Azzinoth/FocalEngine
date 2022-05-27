#pragma once

#include "../Editor/FEEditorSubWindows/selectPopups.h"

class editGameModelPopup : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(editGameModelPopup)

	enum mode
	{
		NO_LOD_MODE = 0,
		HAS_LOD_MODE = 1,
	};
	FEGameModel* objToWorkWith;
	FEGameModel* tempModel = nullptr;
	FETexture* tempPreview = nullptr;

	FEMaterial* updatedMaterial;
	static FEMaterial** materialToModify;
	FEMaterial* updatedBillboardMaterial;
	static FEMaterial** billboardMaterialToModify;

	std::vector<FEMesh*> updatedLODMeshs;

	ImGuiButton* cancelButton;
	ImGuiButton* applyButton;
	ImGuiButton* changeMaterialButton;
	ImGuiButton* changeBillboardMaterialButton;
	ImGuiButton* addBillboard;

	std::vector<ImGuiButton*> changeLODMeshButton;
	ImGuiButton* deleteLODMeshButton;

	int currentMode = NO_LOD_MODE;

	FERangeConfigurator* LODGroups;
	RECT LOD0RangeVisualization;

	std::vector<ImColor> LODColors;

	// ************** Drag&Drop **************
	struct meshTargetCallbackInfo
	{
		int LODLevel;
		editGameModelPopup* window;
	};
	std::vector<meshTargetCallbackInfo> LODsMeshCallbackInfo;
	std::vector<DragAndDropTarget*> LODsMeshTarget;

	struct materialTargetCallbackInfo
	{
		bool billboardMaterial;
		editGameModelPopup* window;
	};
	materialTargetCallbackInfo materialCallbackInfo;
	materialTargetCallbackInfo billboardMaterialCallbackInfo;
	DragAndDropTarget* materialTarget;
	DragAndDropTarget* billboardMaterialTarget;

	static bool dragAndDropLODMeshCallback(FEObject* object, void** callbackInfo);
	static bool dragAndDropMaterialCallback(FEObject* object, void** callbackInfo);
	// ************** Drag&Drop END **************

	bool isLastSetupLOD(size_t LODindex);

	const float NO_LOD_WINDOW_WIDTH = 460.0f;
	const float NO_LOD_WINDOW_HEIGHT = 520.0f;

	static FEMesh** meshToModify;
	static void changeMeshCallBack(std::vector<FEObject*> selectionsResult);
	static void changeMaterialCallBack(std::vector<FEObject*> selectionsResult);
	static void changeBillboardMaterialCallBack(std::vector<FEObject*> selectionsResult);
public:
	SINGLETON_PUBLIC_PART(editGameModelPopup)

	void show(FEGameModel* GameModel);
	void switchMode(int toMode);
	void displayLODGroups();
	void render() override;
	void close();
};

#define USE_NODES

class editMaterialPopup : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(editMaterialPopup)

	ImGuiButton* cancelButton;
	ImGuiImageButton* iconButton = nullptr;
	int textureCount = 0;

	std::vector<std::string> channels = { "r", "g", "b", "a" };
	int textureFromListUnderMouse = -1;
	FETexture* tempContainer = nullptr;
	int textureDestination = -1;

#ifdef USE_NODES
	static FEMaterial* objToWorkWith;

	// ************** Node area **************
	static FEEditorNodeArea* materialNodeArea;

	static ImVec2 windowPosition;
	static ImVec2 nodeGridRelativePosition;
	static ImVec2 mousePositionWhenContextMenuWasOpened;

	static FETexture* textureForNewNode;
	static void nodeSystemMainContextMenu();
	static void textureNodeCreationCallback(std::vector<FEObject*> selectionsResult);
	static void textureNodeCallback(FEEditorNode* node, FE_EDITOR_NODE_EVENT eventWithNode);
	// ************** Node area END **************
	
	// ************** Drag&Drop **************
	struct nodeAreaTargetCallbackInfo
	{
		FETexture* texture;
	};
	nodeAreaTargetCallbackInfo dragAndDropCallbackInfo;
	DragAndDropTarget* nodeAreaTarget;

	FEEditorTextureCreatingNode* textureNode = nullptr;

	static bool dragAndDropnodeAreaTargetCallback(FEObject* object, void** callbackInfo);
	// ************** Drag&Drop END **************
#else
	FEMaterial* objToWorkWith;

	// ************** Drag&Drop **************
	struct materialBindingCallbackInfo
	{
		void** material;
		int textureBinding;
	};
	std::vector<materialBindingCallbackInfo> materialBindingInfo;
	std::vector<DragAndDropTarget*> materialBindingtargets;
	DragAndDropTarget* texturesListTarget;

	static bool dragAndDropCallback(FEObject* object, void** oldTexture);
	static bool dragAndDropTexturesListCallback(FEObject* object, void** material);
	static bool dragAndDropMaterialBindingsCallback(FEObject* object, void** callbackInfoPointer);
	// ************** Drag&Drop END **************
#endif // USE_NODES
public:
	SINGLETON_PUBLIC_PART(editMaterialPopup)

	void show(FEMaterial* material);
	void render() override;
	void close();
};