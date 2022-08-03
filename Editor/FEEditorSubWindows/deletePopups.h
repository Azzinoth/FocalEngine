#pragma once

#include "../FEProject.h"
#include "../NodeSystem/FEVisualNodeSystem/FEVisualNodeSystem.h"  
#include "../NodeSystem/CustomNodes/FEEditorStandardNodes.h"

class DeleteDirectoryPopup;

class DeleteTexturePopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(DeleteTexturePopup)

	friend DeleteDirectoryPopup;
	FETexture* ObjToWorkWith;

	static void DeleteTexture(FETexture* Texture);
public:
	SINGLETON_PUBLIC_PART(DeleteTexturePopup)

	static std::vector<FEMaterial*> MaterialsThatUsesTexture(const FETexture* Texture);
	void Show(FETexture* TextureToDelete);
	void Render() override;
};

class DeleteMeshPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(DeleteMeshPopup)

	friend DeleteDirectoryPopup;
	FEMesh* ObjToWorkWith;

	static void DeleteMesh(FEMesh* Mesh);
public:
	SINGLETON_PUBLIC_PART(DeleteMeshPopup)

	int TimesMeshUsed(const FEMesh* Mesh);
	void Show(FEMesh* MeshToDelete);
	void Render() override;
};

class DeleteMaterialPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(DeleteMaterialPopup)

	friend DeleteDirectoryPopup;
	FEMaterial* ObjToWorkWith;

	static void DeleteMaterial(FEMaterial* Material);
public:
	SINGLETON_PUBLIC_PART(DeleteMaterialPopup)

	int TimesMaterialUsed(const FEMaterial* Material);
	void Show(FEMaterial* Material);
	void Render() override;
};

class DeleteGameModelPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(DeleteGameModelPopup)

	friend DeleteDirectoryPopup;
	FEGameModel* ObjToWorkWith;

	static void DeleteGameModel(FEGameModel* GameModel);
public:
	SINGLETON_PUBLIC_PART(DeleteGameModelPopup)

	int TimesGameModelUsed(const FEGameModel* GameModel);
	void Show(FEGameModel* GameModel);
	void Render() override;
};

class DeletePrefabPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(DeletePrefabPopup)

	friend DeleteDirectoryPopup;
	FEPrefab* ObjToWorkWith;

	static void DeletePrefab(FEPrefab* Prefab);
public:
	SINGLETON_PUBLIC_PART(DeletePrefabPopup)

	int TimesPrefabUsed(const FEPrefab* Prefab);
	void Show(FEPrefab* Prefab);
	void Render() override;
};

class DeleteDirectoryPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(DeleteDirectoryPopup)

	std::string ObjToWorkWith;
	std::string PathToDirectory;
public:
	SINGLETON_PUBLIC_PART(DeleteDirectoryPopup)

	void Show(std::string DirectoryName);
	void RecursiveDeletion(std::string Path);
	void Render() override;
};