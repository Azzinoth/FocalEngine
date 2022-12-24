#pragma once

#include "resizeTexturePopup.h"

class SelectFeObjectPopUp : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(SelectFeObjectPopUp)

	int IndexUnderMouse = -1;
	FEObject* HighlightedObject = nullptr;

	std::vector<FEObject*> ItemsList;
	std::vector<FEObject*> FilteredItemsList;
	char Filter[512];

	ImGuiButton* SelectButton = nullptr;
	ImGuiButton* CancelButton = nullptr;
	ImGuiImageButton* IconButton = nullptr;

	void(*CallBack)(std::vector<FEObject*>) = nullptr;
	std::vector<FEObject*> SelectedObjects;
	void OnSelectAction();

	FE_OBJECT_TYPE CurrenType;

	static void KeyButtonCallback(int Key, int Scancode, int Action, int Mods);
	static bool ControlButtonPressed;
	static bool bOneObjectSelectonMode;
	bool IsSelected(const FEObject* Object) const;
	void AddToSelected(FEObject* Object);
public:
	SINGLETON_PUBLIC_PART(SelectFeObjectPopUp)

	void Show(FE_OBJECT_TYPE Type, void(*CallBack)(std::vector<FEObject*>), FEObject* HighlightedObject = nullptr, std::vector<FEObject*> CustomList = std::vector<FEObject*>());
	void Close() override;
	void Render() override;

	bool IsOneObjectSelectonMode();
	void SetOneObjectSelectonMode(bool NewValue);
};