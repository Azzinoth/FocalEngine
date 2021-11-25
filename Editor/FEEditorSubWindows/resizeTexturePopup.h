#pragma once

#include "../Editor/FEEditorSubWindows/renamePopups.h"

class resizeTexturePopup : public ImGuiModalPopup
{
	FETexture* objToWorkWith;
	int newWidth;
	int newHeight;
	int filtration;
	bool replaceTexture = false;

	ImGuiButton* cancelButton;
	ImGuiButton* applyButton;
	ImVec2 popupSize = ImVec2(450, 260);
public:
	SINGLETON_PUBLIC_PART(resizeTexturePopup)

	void show(FETexture* ObjToWorkWith);
	void render() override;
private:
	SINGLETON_PRIVATE_PART(resizeTexturePopup)
};