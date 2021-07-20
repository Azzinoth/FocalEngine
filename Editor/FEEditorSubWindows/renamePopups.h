#pragma once

#include "../Editor/FEEditorSubWindows/deletePopups.h"

class renameFailedPopUp : public ImGuiModalPopup
{
	ImGuiButton* okButton = nullptr;
public:
	SINGLETON_PUBLIC_PART(renameFailedPopUp)
	void render() override;
private:
	SINGLETON_PRIVATE_PART(renameFailedPopUp)
};

class renamePopUp : public ImGuiModalPopup
{
	FEObject* objToWorkWith;
	char newName[512];
public:
	SINGLETON_PUBLIC_PART(renamePopUp)

	void show(FEObject* ObjToWorkWith);
	void render() override;
private:
	SINGLETON_PRIVATE_PART(renamePopUp)
};