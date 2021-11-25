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

private:
	SINGLETON_PUBLIC_PART(FEEditorPreviewManager)
	SINGLETON_PRIVATE_PART(FEEditorPreviewManager)

	void initializeResources();
	void updateAll();

	FEFramebuffer* previewFB;
	FEEntity* previewEntity;
	FEGameModel* previewGameModel;
	FEMaterial* meshPreviewMaterial;

	std::unordered_map<std::string, FETexture*> meshPreviewTextures;
	std::unordered_map<std::string, FETexture*> materialPreviewTextures;
	std::unordered_map<std::string, FETexture*> gameModelPreviewTextures;

	void createMeshPreview(std::string meshID);
	FETexture* getMeshPreview(std::string meshID);

	void createMaterialPreview(std::string materialID);
	FETexture* getMaterialPreview(std::string materialID);

	void createGameModelPreview(std::string gameModelID);
	void createGameModelPreview(FEGameModel* gameModel, FETexture** resultingTexture);
	FETexture* getGameModelPreview(std::string gameModelID);
	void updateAllGameModelPreviews();

	void clear();
};

#define PREVIEW_MANAGER FEEditorPreviewManager::getInstance()