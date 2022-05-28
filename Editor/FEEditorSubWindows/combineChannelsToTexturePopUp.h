#pragma once

#include "../Editor/FEEditorSubWindows/gyzmosSettingsWindow.h"

class CombineChannelsToTexturePopUp : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(CombineChannelsToTexturePopUp)

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

	// ************** Node area **************
	static FEVisualNodeArea* currentNodeArea;

	static FETexture* textureForNewNode;
	static void nodeSystemMainContextMenu();
	static void textureNodeCreationCallback(std::vector<FEObject*> selectionsResult);

	static ImVec2 windowPosition;
	static ImVec2 nodeGridRelativePosition;
	static ImVec2 mousePositionWhenContextMenuWasOpened;
	// ************** Node area END **************
public:
	SINGLETON_PUBLIC_PART(CombineChannelsToTexturePopUp)

	void show(std::string FilePath);
	void render() override;
};