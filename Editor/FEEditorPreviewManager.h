#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEEditor;
class FEProjectManager;
class DeleteTexturePopup;
class RenameMeshPopUp;
class DeleteMeshPopup;
class SelectMeshPopUp;
class SelectMaterialPopUp;
class SelectGameModelPopUp;
class EditGameModelPopup;
class EditMaterialPopup;
class DeleteMaterialPopup;
class SelectFeObjectPopUp;
class PrefabEditorWindow;

class FEEditorPreviewManager
{
	friend FEEditor;
	friend FEProjectManager;
	friend DeleteTexturePopup;
	friend RenameMeshPopUp;
	friend DeleteMeshPopup;
	friend SelectMeshPopUp;
	friend SelectMaterialPopUp;
	friend SelectGameModelPopUp;
	friend EditGameModelPopup;
	friend EditMaterialPopup;
	friend DeleteMaterialPopup;
	friend SelectFeObjectPopUp;
	friend PrefabEditorWindow;

private:
	SINGLETON_PUBLIC_PART(FEEditorPreviewManager)
	SINGLETON_PRIVATE_PART(FEEditorPreviewManager)

	void InitializeResources();
	void UpdateAll();

	FEFramebuffer* PreviewFB;
	FEEntity* PreviewEntity;
	FEPrefab* PreviewPrefab;
	FEGameModel* PreviewGameModel;
	FEMaterial* MeshPreviewMaterial;

	std::unordered_map<std::string, FETexture*> MeshPreviewTextures;
	std::unordered_map<std::string, FETexture*> MaterialPreviewTextures;
	std::unordered_map<std::string, FETexture*> GameModelPreviewTextures;
	std::unordered_map<std::string, FETexture*> PrefabPreviewTextures;

	void CreateMeshPreview(std::string MeshID);
	FETexture* GetMeshPreview(std::string MeshID);

	void CreateMaterialPreview(std::string MaterialID);
	FETexture* GetMaterialPreview(std::string MaterialID);

	void CreateGameModelPreview(std::string GameModelID);
	void CreateGameModelPreview(const FEGameModel* GameModel, FETexture** ResultingTexture) const;
	FETexture* GetGameModelPreview(std::string GameModelID);
	void UpdateAllGameModelPreviews();

	void CreatePrefabPreview(std::string PrefabID);
	void CreatePrefabPreview(FEPrefab* Prefab, FETexture** ResultingTexture);
	FETexture* GetPrefabPreview(std::string PrefabID);

	FETexture* GetPreview(FEObject* Object);
	FETexture* GetPreview(std::string ObjectID);

	void Clear();
};

#define PREVIEW_MANAGER FEEditorPreviewManager::getInstance()