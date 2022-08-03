#pragma once

#include "combineChannelsToTexturePopUp.h"

class PrefabEditorWindow : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(PrefabEditorWindow)

	// ************** Drag&Drop **************
	DragAndDropTarget* AddGameModelTarget = nullptr;
	static bool AddGameModelTargetCallBack(FEObject* Object, void** EntityPointer);
	// ************** Drag&Drop END **************

	ImGuiButton* CloseButton = nullptr;

	static FEPrefab* ObjToWorkWith;
	int HoveredGameModelItem = -1;
	int SelectedGameModel = -1;

	void ShowTransformConfiguration(FETransformComponent* Transform, int Index);
	static void AddNewGameModelCallBack(std::vector<FEObject*> SelectionsResult);
public:
	SINGLETON_PUBLIC_PART(PrefabEditorWindow)

	void Show(FEPrefab* Prefab);
	void Render() override;
};