#pragma once

#include "combineChannelsToTexturePopUp.h"

class prefabEditorWindow : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(prefabEditorWindow)

	// ************** Drag&Drop **************
	DragAndDropTarget* addGameModelTarget = nullptr;
	static bool addGameModelTargetCallBack(FEObject* object, void** entityPointer);
	// ************** Drag&Drop END **************

	ImGuiButton* closeButton = nullptr;

	static FEPrefab* objToWorkWith;
	int hoveredGameModelItem = -1;
	int selectedGameModel = -1;

	void showTransformConfiguration(FETransformComponent* transform, int index);
	static void addNewGameModelCallBack(std::vector<FEObject*> selectionsResult);
public:
	SINGLETON_PUBLIC_PART(prefabEditorWindow)

	void show(FEPrefab* Prefab);
	void render() override;
};