#pragma once

#include "renamePopups.h"

class resizeTexturePopup : public ImGuiModalPopup
{
	FETexture* ObjToWorkWith;
	int NewWidth;
	int NewHeight;
	int Filtration;
	bool bReplaceTexture = false;

	ImGuiButton* CancelButton;
	ImGuiButton* ApplyButton;
	ImVec2 PopupSize = ImVec2(450, 260);
public:
	SINGLETON_PUBLIC_PART(resizeTexturePopup)

	void Show(FETexture* ObjToWorkWith);
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(resizeTexturePopup)
};