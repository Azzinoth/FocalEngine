#pragma once

#include "selectPopups.h"

class EditGameModelPopup : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(EditGameModelPopup)

	enum EDIT_GAME_MODEL_LOD_MODE
	{
		NO_LOD_MODE = 0,
		HAS_LOD_MODE = 1,
	};

	FEGameModel* ObjToWorkWith;
	FEGameModel* TempModel = nullptr;
	FETexture* TempPreview = nullptr;

	FEMaterial* UpdatedMaterial;
	static FEMaterial** MaterialToModify;
	FEMaterial* UpdatedBillboardMaterial;
	static FEMaterial** BillboardMaterialToModify;

	std::vector<FEMesh*> UpdatedLODMeshs;

	ImGuiButton* CancelButton;
	ImGuiButton* ApplyButton;
	ImGuiButton* ChangeMaterialButton;
	ImGuiButton* ChangeBillboardMaterialButton;
	ImGuiButton* AddBillboard;

	std::vector<ImGuiButton*> ChangeLODMeshButton;
	ImGuiButton* DeleteLODMeshButton;

	int CurrentMode = NO_LOD_MODE;

	FERangeConfigurator* LODGroups;
	RECT LOD0RangeVisualization;

	std::vector<ImColor> LODColors;

	// ************** Drag&Drop **************
	struct MeshTargetCallbackInfo
	{
		int LODLevel;
		EditGameModelPopup* Window;
	};
	std::vector<MeshTargetCallbackInfo> LODMeshCallbackInfo;
	std::vector<DragAndDropTarget*> LODMeshTarget;

	struct MaterialTargetCallbackInfo
	{
		bool bBillboardMaterial;
		EditGameModelPopup* Window;
	};
	MaterialTargetCallbackInfo MaterialCallbackInfo;
	MaterialTargetCallbackInfo BillboardMaterialCallbackInfo;
	DragAndDropTarget* MaterialTarget;
	DragAndDropTarget* BillboardMaterialTarget;

	static bool DragAndDropLODMeshCallback(FEObject* Object, void** CallbackInfo);
	static bool DragAndDropMaterialCallback(FEObject* Object, void** CallbackInfo);
	// ************** Drag&Drop END **************

	bool IsLastSetupLOD(size_t LODIndex);

	const float NO_LOD_WINDOW_WIDTH = 460.0f;
	const float NO_LOD_WINDOW_HEIGHT = 520.0f;

	static FEMesh** MeshToModify;
	static void ChangeMeshCallBack(std::vector<FEObject*> SelectionsResult);
	static void ChangeMaterialCallBack(std::vector<FEObject*> SelectionsResult);
	static void ChangeBillboardMaterialCallBack(std::vector<FEObject*> SelectionsResult);
public:
	SINGLETON_PUBLIC_PART(EditGameModelPopup)

	void Show(FEGameModel* GameModel);
	void SwitchMode(int ToMode);
	void DisplayLODGroups();
	void Render() override;
	void Close();
};

#define USE_NODES

class EditMaterialPopup : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(EditMaterialPopup)

	ImGuiButton* CancelButton;
	ImGuiImageButton* IconButton = nullptr;
	int TextureCount = 0;

	std::vector<std::string> Channels = { "r", "g", "b", "a" };
	int TextureFromListUnderMouse = -1;
	FETexture* TempContainer = nullptr;
	int TextureDestination = -1;

#ifdef USE_NODES
	static FEMaterial* ObjToWorkWith;

	// ************** Node area **************
	static FEVisualNodeArea* MaterialNodeArea;

	static ImVec2 WindowPosition;
	static ImVec2 NodeGridRelativePosition;
	static ImVec2 MousePositionWhenContextMenuWasOpened;

	static FETexture* TextureForNewNode;
	static void NodeSystemMainContextMenu();
	static void TextureNodeCreationCallback(std::vector<FEObject*> SelectionsResult);
	static void TextureNodeCallback(FEVisualNode* Node, FE_VISUAL_NODE_EVENT EventWithNode);
	// ************** Node area END **************
	
	// ************** Drag&Drop **************
	struct NodeAreaTargetCallbackInfo
	{
		FETexture* Texture;
	};
	NodeAreaTargetCallbackInfo DragAndDropCallbackInfo;
	DragAndDropTarget* NodeAreaTarget;

	FEEditorTextureCreatingNode* TextureNode = nullptr;

	static bool DragAndDropnodeAreaTargetCallback(FEObject* Object, void** CallbackInfo);
	// ************** Drag&Drop END **************
#else
	FEMaterial* ObjToWorkWith;

	// ************** Drag&Drop **************
	struct MaterialBindingCallbackInfo
	{
		void** Material;
		int TextureBinding;
	};
	std::vector<MaterialBindingCallbackInfo> MaterialBindingInfo;
	std::vector<DragAndDropTarget*> MaterialBindingtargets;
	DragAndDropTarget* TexturesListTarget;

	static bool DragAndDropCallback(FEObject* Object, void** OldTexture);
	static bool DragAndDropTexturesListCallback(FEObject* Object, void** Material);
	static bool DragAndDropMaterialBindingsCallback(FEObject* Object, void** CallbackInfoPointer);
	// ************** Drag&Drop END **************
#endif // USE_NODES
public:
	SINGLETON_PUBLIC_PART(EditMaterialPopup)

	void Show(FEMaterial* Material);
	void Render() override;
	void Close();
};