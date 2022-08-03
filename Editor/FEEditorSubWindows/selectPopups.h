#pragma once

#include "../Editor/FEEditorSubWindows/resizeTexturePopup.h"

class SelectFeObjectPopUp : public ImGuiModalPopup
{
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
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
	bool IsSelected(const FEObject* Object) const;
public:
	SINGLETON_PUBLIC_PART(SelectFeObjectPopUp)

	void Show(FE_OBJECT_TYPE Type, void(*CallBack)(std::vector<FEObject*>), FEObject* HighlightedObject = nullptr, std::vector<FEObject*> CustomList = std::vector<FEObject*>());
	void Close() override;
	void Render() override;

private:
	SINGLETON_PRIVATE_PART(SelectFeObjectPopUp)
};