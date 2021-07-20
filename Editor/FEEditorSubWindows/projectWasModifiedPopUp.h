#pragma once

#include "../Editor/FEEditorSubWindows/loadTexturePopup.h"

static bool shouldTerminate = false;
class projectWasModifiedPopUp : public ImGuiModalPopup
{
	FEProject* objToWorkWith;
public:
	SINGLETON_PUBLIC_PART(projectWasModifiedPopUp)

	void show(FEProject* project);
	void render() override;

private:
	SINGLETON_PRIVATE_PART(projectWasModifiedPopUp)
};