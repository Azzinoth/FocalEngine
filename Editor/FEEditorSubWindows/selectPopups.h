#pragma once

#include "../Editor/FEEditorSubWindows/resizeTexturePopup.h"

class selectFEObjectPopUp : public ImGuiModalPopup
{
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	FEObject* highlightedObject = nullptr;

	std::vector<FEObject*> itemsList;
	std::vector<FEObject*> filteredItemsList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;

	void(*callBack)(std::vector<FEObject*>) = nullptr;
	std::vector<FEObject*> selectedObjects;
	void onSelectAction();

	FEObjectType currenType;

	static void keyButtonCallback(int key, int scancode, int action, int mods);
	static bool controlButtonPressed;
	bool isSelected(FEObject* object);
public:
	SINGLETON_PUBLIC_PART(selectFEObjectPopUp)

	void show(FEObjectType type, void(*CallBack)(std::vector<FEObject*>), FEObject* HighlightedObject = nullptr, std::vector<FEObject*> customList = std::vector<FEObject*>());
	void close() override;
	void render() override;

private:
	SINGLETON_PRIVATE_PART(selectFEObjectPopUp)
};