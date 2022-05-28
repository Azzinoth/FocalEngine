#pragma once

#include "../FEProject.h"
#include "../NodeSystem/FEVisualNodeSystem/FEVisualNodeSystem.h"  
#include "../NodeSystem/CustomNodes/FEEditorStandardNodes.h"

class deleteDirectoryPopup;

class deleteTexturePopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(deleteTexturePopup)

	friend deleteDirectoryPopup;
	FETexture* objToWorkWith;

	static void deleteTexture(FETexture* texture);
public:
	SINGLETON_PUBLIC_PART(deleteTexturePopup)

	static std::vector<FEMaterial*> materialsThatUsesTexture(FETexture* texture);
	void show(FETexture* TextureToDelete);
	void render() override;
};

class deleteMeshPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(deleteMeshPopup)

	friend deleteDirectoryPopup;
	FEMesh* objToWorkWith;

	static void deleteMesh(FEMesh* mesh);
public:
	SINGLETON_PUBLIC_PART(deleteMeshPopup)

	int timesMeshUsed(FEMesh* mesh);
	void show(FEMesh* MeshToDelete);
	void render() override;
};

class deleteMaterialPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(deleteMaterialPopup)

		friend deleteDirectoryPopup;
	FEMaterial* objToWorkWith;

	static void deleteMaterial(FEMaterial* material);
public:
	SINGLETON_PUBLIC_PART(deleteMaterialPopup)

		int timesMaterialUsed(FEMaterial* material);
	void show(FEMaterial* material);
	void render() override;
};

class deleteGameModelPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(deleteGameModelPopup)

	friend deleteDirectoryPopup;
	FEGameModel* objToWorkWith;

	static void deleteGameModel(FEGameModel* gameModel);
public:
	SINGLETON_PUBLIC_PART(deleteGameModelPopup)

	int timesGameModelUsed(FEGameModel* gameModel);
	void show(FEGameModel* GameModel);
	void render() override;
};

class deletePrefabPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(deletePrefabPopup)

	friend deleteDirectoryPopup;
	FEPrefab* objToWorkWith;

	static void deletePrefab(FEPrefab* Prefab);
public:
	SINGLETON_PUBLIC_PART(deletePrefabPopup)

	int timesPrefabUsed(FEPrefab* Prefab);
	void show(FEPrefab* Prefab);
	void render() override;
};

class deleteDirectoryPopup : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(deleteDirectoryPopup)

	std::string objToWorkWith;
	std::string pathToDirectory;
public:
	SINGLETON_PUBLIC_PART(deleteDirectoryPopup)

	void show(std::string directoryName);
	void recursiveDeletion(std::string path);
	void render() override;
};