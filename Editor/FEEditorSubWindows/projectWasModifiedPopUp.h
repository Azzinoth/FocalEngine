#pragma once

#include "../Editor/FEEditorSubWindows/combineChannelsToTexturePopUp.h"

class projectWasModifiedPopUp : public ImGuiModalPopup
{
	FEProject* objToWorkWith;
	bool shouldTerminate = false;
public:
	SINGLETON_PUBLIC_PART(projectWasModifiedPopUp)

	void show(FEProject* project, bool fullyCloseApplication);
	void render() override;

private:
	SINGLETON_PRIVATE_PART(projectWasModifiedPopUp)
};