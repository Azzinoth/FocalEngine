#pragma once

#include "../Editor/FEEditorSubWindows/debugTextureViewWindow.h"

class projectWasModifiedPopUp : public ImGuiModalPopup
{
	FEProject* ObjToWorkWith;
	bool bShouldTerminate = false;
public:
	SINGLETON_PUBLIC_PART(projectWasModifiedPopUp)

	void Show(FEProject* Project, bool FullyCloseApplication);
	void Render() override;

private:
	SINGLETON_PRIVATE_PART(projectWasModifiedPopUp)
};