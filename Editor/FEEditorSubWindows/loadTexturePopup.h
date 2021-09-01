#pragma once

#include "../Editor/FEEditorSubWindows/gyzmosSettingsWindow.h"

class loadTexturePopUp : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(loadTexturePopUp)

	std::string filePath;
public:
	SINGLETON_PUBLIC_PART(loadTexturePopUp)

	void show(std::string FilePath);
	void render() override;
};

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
	static FEEditorNodeArea* currentNodeArea;

	static FETexture* textureForNewNode;
	static void nodeSystemMainContextMenu();
	static void textureNodeCreationCallback(void* texture);

	static ImVec2 windowPosition;
	static ImVec2 nodeGridRelativePosition;
	static ImVec2 mousePositionWhenContextMenuWasOpened;
	// ************** Node area END **************
public:
	SINGLETON_PUBLIC_PART(CombineChannelsToTexturePopUp)

	void show(std::string FilePath);
	void render() override;
};