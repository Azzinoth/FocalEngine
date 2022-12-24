#pragma once

#include "deletePopups.h"

class renameFailedPopUp : public ImGuiModalPopup
{
	ImGuiButton* OkButton = nullptr;
public:
	SINGLETON_PUBLIC_PART(renameFailedPopUp)
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(renameFailedPopUp)
};

class renamePopUp : public ImGuiModalPopup
{
	FEObject* ObjToWorkWith;
	char NewName[512];
public:
	SINGLETON_PUBLIC_PART(renamePopUp)

	void Show(FEObject* ObjToWorkWith);
	void Render() override;
private:
	SINGLETON_PRIVATE_PART(renamePopUp)
};