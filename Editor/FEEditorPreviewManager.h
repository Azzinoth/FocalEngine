#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEEditor;
class FEProjectManager;
class deleteTexturePopup;
class renameMeshPopUp;
class deleteMeshPopup;
class selectMeshPopUp;
class selectMaterialPopUp;
class selectGameModelPopUp;
class editGameModelPopup;
class editMaterialPopup;
class deleteMaterialPopup;
class selectFEObjectPopUp;
class prefabEditorWindow;

class FEEditorPreviewManager
{
	friend FEEditor;
	friend FEProjectManager;
	friend deleteTexturePopup;
	friend renameMeshPopUp;
	friend deleteMeshPopup;
	friend selectMeshPopUp;
	friend selectMaterialPopUp;
	friend selectGameModelPopUp;
	friend editGameModelPopup;
	friend editMaterialPopup;
	friend deleteMaterialPopup;
	friend selectFEObjectPopUp;
	friend prefabEditorWindow;

private:
	SINGLETON_PUBLIC_PART(FEEditorPreviewManager)
	SINGLETON_PRIVATE_PART(FEEditorPreviewManager)

	void initializeResources();
	void updateAll();

	FEFramebuffer* previewFB;
	FEEntity* previewEntity;
	FEPrefab* previewPrefab;
	FEGameModel* previewGameModel;
	FEMaterial* meshPreviewMaterial;

	std::unordered_map<std::string, FETexture*> meshPreviewTextures;
	std::unordered_map<std::string, FETexture*> materialPreviewTextures;
	std::unordered_map<std::string, FETexture*> gameModelPreviewTextures;
	std::unordered_map<std::string, FETexture*> prefabPreviewTextures;

	void createMeshPreview(std::string meshID);
	FETexture* getMeshPreview(std::string meshID);

	void createMaterialPreview(std::string materialID);
	FETexture* getMaterialPreview(std::string materialID);

	void createGameModelPreview(std::string gameModelID);
	void createGameModelPreview(FEGameModel* gameModel, FETexture** resultingTexture);
	FETexture* getGameModelPreview(std::string gameModelID);
	void updateAllGameModelPreviews();

	void createPrefabPreview(std::string prefabID);
	void createPrefabPreview(FEPrefab* prefab, FETexture** resultingTexture);
	FETexture* getPrefabPreview(std::string prefabID);

	FETexture* getPreview(FEObject* FEObject);
	FETexture* getPreview(std::string FEObjectID);

	void clear();
};

#define PREVIEW_MANAGER FEEditorPreviewManager::getInstance()