#pragma once

#include "../Editor/FEEditorSubWindows/renamePopups.h"

template <class FEOBJECT>
struct contenetBrowserItem
{
	FEObject* FEObjectPart;
	void* pointerToObject;

	contenetBrowserItem(FEOBJECT* FEObject)
	{
		FEObjectPart = FEObject;
		pointerToObject = FEObject;
	}
};

class selectMeshPopUp : public ImGuiModalPopup
{
	FEMesh** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemID = "";
	std::vector<contenetBrowserItem<FEMesh>> itemsList;
	std::vector<contenetBrowserItem<FEMesh>> filteredItemsList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;
public:
	SINGLETON_PUBLIC_PART(selectMeshPopUp)

	void show(FEMesh** mesh);
	void close() override;
	void render() override;
private:
	SINGLETON_PRIVATE_PART(selectMeshPopUp)
};

class selectTexturePopUp : public ImGuiModalPopup
{
	FETexture** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemID = "";
	std::vector<contenetBrowserItem<FETexture>> itemsList;
	std::vector<contenetBrowserItem<FETexture>> filteredItemsList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;

	void(*onSelect)(void*) = nullptr;
	void* ptrOnClose = nullptr;
public:
	SINGLETON_PUBLIC_PART(selectTexturePopUp)

	void show(FETexture** texture, void(*func)(void*) = nullptr, void* ptr = nullptr);
	void showWithCustomList(FETexture** texture, std::vector<FETexture*> customList);
	void close() override;
	void onSelectAction();
	void render() override;

private:
	SINGLETON_PRIVATE_PART(selectTexturePopUp)
};

class selectMaterialPopUp : public ImGuiModalPopup
{
	FEMaterial** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemID = "";
	std::vector<contenetBrowserItem<FEMaterial>> itemsList;
	std::vector<contenetBrowserItem<FEMaterial>> filteredItemsList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;

	FEShader* allowedShader = nullptr;
public:
	SINGLETON_PUBLIC_PART(selectMaterialPopUp)

	void setAllowedShader(FEShader* shader);
	void show(FEMaterial** material);

	void close() override;
	void render() override;

private:
	SINGLETON_PRIVATE_PART(selectMaterialPopUp)
};

class selectGameModelPopUp : public ImGuiModalPopup
{
	FEGameModel** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemID = "";
	std::vector<contenetBrowserItem<FEGameModel>> itemsList;
	std::vector<contenetBrowserItem<FEGameModel>> filteredItemsList;
	char filter[512];
	bool newEntityFlag = false;
	bool isInstanced = false;
	bool wasSelectedAlready = false;

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;
public:
	SINGLETON_PUBLIC_PART(selectGameModelPopUp)

	void show(FEGameModel** gameModel, bool newEntityFlag = false, bool isInstanced = false);
	void close() override;
	void render() override;

private:
	SINGLETON_PRIVATE_PART(selectGameModelPopUp)
};