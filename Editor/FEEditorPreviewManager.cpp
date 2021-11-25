#include "FEEditorPreviewManager.h"
using namespace FocalEngine;

FEEditorPreviewManager* FEEditorPreviewManager::_instance = nullptr;
FEEditorPreviewManager::FEEditorPreviewManager() {}
FEEditorPreviewManager::~FEEditorPreviewManager() {}

void FEEditorPreviewManager::initializeResources()
{
	previewFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, 128, 128);
	previewGameModel = new FEGameModel(nullptr, nullptr, "editorPreviewGameModel");
	previewEntity = new FEEntity(previewGameModel, "editorPreviewEntity");
	meshPreviewMaterial = RESOURCE_MANAGER.createMaterial("meshPreviewMaterial");
	RESOURCE_MANAGER.makeMaterialStandard(meshPreviewMaterial);
	meshPreviewMaterial->shader = RESOURCE_MANAGER.createShader("FEMeshPreviewShader", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_MeshPreview_VS.glsl").c_str(),
																					   RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_MeshPreview_FS.glsl").c_str(),
																					   nullptr,
																					   nullptr,
																					   nullptr,
																					   nullptr,
																					   "607A53601357077F03770357"/*"FEMeshPreviewShader"*/);

	RESOURCE_MANAGER.makeShaderStandard(meshPreviewMaterial->shader);
}

void FEEditorPreviewManager::updateAll()
{
	clear();

	std::vector<std::string> meshList = RESOURCE_MANAGER.getMeshList();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		createMeshPreview(meshList[i]);
	}

	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();
	for (size_t i = 0; i < materialList.size(); i++)
	{
		createMaterialPreview(materialList[i]);
	}

	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		createGameModelPreview(gameModelList[i]);
	}
}

void FEEditorPreviewManager::createMeshPreview(std::string meshID)
{
	FEMesh* previewMesh = RESOURCE_MANAGER.getMesh(meshID);
	if (previewMesh == nullptr)
		return;

	previewGameModel->mesh = previewMesh;
	previewGameModel->material = meshPreviewMaterial;

	previewFB->bind();
	// We use this values even with deffered renderer because final image will not be gamma corrected, so values over 1.0f would not work.
	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	FocalEngine::FETransformComponent regularMeshTransform = previewEntity->transform;
	glm::vec3 regularCameraPosition = ENGINE.getCamera()->getPosition();
	float regularAspectRation = ENGINE.getCamera()->getAspectRatio();
	float regularCameraPitch = ENGINE.getCamera()->getPitch();
	float regularCameraRoll = ENGINE.getCamera()->getRoll();
	float regularCameraYaw = ENGINE.getCamera()->getYaw();

	// transform has influence on AABB, so before any calculations setting needed values
	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(15.0, -15.0, 0.0));

	FEAABB meshAABB = previewEntity->getAABB();
	glm::vec3 min = meshAABB.getMin();
	glm::vec3 max = meshAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	previewEntity->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	ENGINE.getCamera()->setPosition(glm::vec3(0.0, 0.0, std::max(std::max(xSize, ySize), zSize) * 1.75f));

	ENGINE.getCamera()->setAspectRatio(1.0f);
	FE_GL_ERROR(glViewport(0, 0, 128, 128));

	ENGINE.getCamera()->setPitch(0.0f);
	ENGINE.getCamera()->setRoll(0.0f);
	ENGINE.getCamera()->setYaw(0.0f);

	// rendering mesh to texture
	RENDERER.renderEntity(previewEntity, ENGINE.getCamera());

	// reversing all of our transformating.
	previewEntity->transform = regularMeshTransform;

	ENGINE.getCamera()->setPosition(regularCameraPosition);
	ENGINE.getCamera()->setAspectRatio(regularAspectRation);
	ENGINE.getCamera()->setPitch(regularCameraPitch);
	ENGINE.getCamera()->setRoll(regularCameraRoll);
	ENGINE.getCamera()->setYaw(regularCameraYaw);

	previewFB->unBind();

	glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w);

	// if we are updating preview we should delete old texture.
	if (meshPreviewTextures.find(meshID) != meshPreviewTextures.end())
		delete meshPreviewTextures[meshID];

	meshPreviewTextures[meshID] = previewFB->getColorAttachment();
	previewFB->setColorAttachment(RESOURCE_MANAGER.createSameFormatTexture(previewFB->getColorAttachment()));
}

FETexture* FEEditorPreviewManager::getMeshPreview(std::string meshID)
{
	// if mesh's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.getMesh(meshID)->getDirtyFlag())
	{
		createMeshPreview(meshID);
		// if some game model uses this mesh we should also update it's preview
		std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();

		for (size_t i = 0; i < gameModelList.size(); i++)
		{
			FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);

			if (currentGameModel->mesh == RESOURCE_MANAGER.getMesh(meshID))
				createGameModelPreview(currentGameModel->getObjectID());
		}

		RESOURCE_MANAGER.getMesh(meshID)->setDirtyFlag(false);
	}

	// if we somehow could not find preview, we will create it.
	if (meshPreviewTextures.find(meshID) == meshPreviewTextures.end())
		createMeshPreview(meshID);

	// if still we don't have it
	if (meshPreviewTextures.find(meshID) == meshPreviewTextures.end())
		return RESOURCE_MANAGER.noTexture;

	return meshPreviewTextures[meshID];
}

void FEEditorPreviewManager::createMaterialPreview(std::string materialID)
{
	FEMaterial* previewMaterial = RESOURCE_MANAGER.getMaterial(materialID);
	if (previewMaterial == nullptr)
		return;

	FEDirectionalLight* currentDirectionalLight = nullptr;
	std::vector<std::string> lightList = SCENE.getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (SCENE.getLight(lightList[i])->getType() == FE_DIRECTIONAL_LIGHT)
		{
			currentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.getLight(lightList[i]));
			break;
		}
	}
	glm::vec3 regularLightRotation = currentDirectionalLight->transform.getRotation();
	currentDirectionalLight->transform.setRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	float regularLightIntensity = currentDirectionalLight->getIntensity();
	currentDirectionalLight->setIntensity(10.0f);

	previewGameModel->mesh = RESOURCE_MANAGER.getMesh("7F251E3E0D08013E3579315F"/*"sphere"*/);
	previewGameModel->material = previewMaterial;
	previewEntity->setReceivingShadows(false);

	previewFB->bind();
	// We use this values even with deffered renderer because final image will not be gamma corrected, so values over 1.0f would not work.
	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	glm::vec3 regularCameraPosition = ENGINE.getCamera()->getPosition();
	float regularAspectRation = ENGINE.getCamera()->getAspectRatio();
	float regularCameraPitch = ENGINE.getCamera()->getPitch();
	float regularCameraRoll = ENGINE.getCamera()->getRoll();
	float regularCameraYaw = ENGINE.getCamera()->getYaw();
	float regularExposure = ENGINE.getCamera()->getExposure();
	ENGINE.getCamera()->setExposure(1.0f);

	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(0.0, 0.0, 0.0));

	ENGINE.getCamera()->setPosition(glm::vec3(0.0, 0.0, 70.0f));
	ENGINE.getCamera()->setAspectRatio(1.0f);
	FE_GL_ERROR(glViewport(0, 0, 128, 128));

	ENGINE.getCamera()->setPitch(0.0f);
	ENGINE.getCamera()->setRoll(0.0f);
	ENGINE.getCamera()->setYaw(0.0f);

	// rendering material to texture
#ifdef USE_DEFERRED_RENDERER
	RENDERER.renderEntityForward(previewEntity, ENGINE.getCamera(), true);
#else
	RENDERER.renderEntity(previewEntity, ENGINE.getCamera(), true);
#endif // USE_DEFERRED_RENDERER

	ENGINE.getCamera()->setPosition(regularCameraPosition);
	ENGINE.getCamera()->setAspectRatio(regularAspectRation);
	ENGINE.getCamera()->setPitch(regularCameraPitch);
	ENGINE.getCamera()->setRoll(regularCameraRoll);
	ENGINE.getCamera()->setYaw(regularCameraYaw);
	ENGINE.getCamera()->setExposure(regularExposure);

	currentDirectionalLight->transform.setRotation(regularLightRotation);
	currentDirectionalLight->setIntensity(regularLightIntensity);

	previewFB->unBind();

	glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w);

	// if we are updating preview we should delete old texture.
	if (materialPreviewTextures.find(materialID) != materialPreviewTextures.end())
		delete materialPreviewTextures[materialID];

	materialPreviewTextures[materialID] = previewFB->getColorAttachment();
	previewFB->setColorAttachment(RESOURCE_MANAGER.createSameFormatTexture(previewFB->getColorAttachment()));

	// looking for all gameModels that uses this material to also update them
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
		if (currentGameModel->material == previewMaterial && currentGameModel != previewGameModel)
			createGameModelPreview(currentGameModel->getName());
	}
}

FETexture* FEEditorPreviewManager::getMaterialPreview(std::string materialID)
{
	// if material's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.getMaterial(materialID)->getDirtyFlag())
	{
		createMaterialPreview(materialID);
		// if some game model uses this material we should also update it's preview
		std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();

		for (size_t i = 0; i < gameModelList.size(); i++)
		{
			FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);

			if (currentGameModel->material == RESOURCE_MANAGER.getMaterial(materialID))
				createGameModelPreview(currentGameModel->getObjectID());
		}

		RESOURCE_MANAGER.getMaterial(materialID)->setDirtyFlag(false);
	}	

	// if we somehow could not find preview, we will create it.
	if (materialPreviewTextures.find(materialID) == materialPreviewTextures.end())
		createMaterialPreview(materialID);

	// if still we don't have it
	if (materialPreviewTextures.find(materialID) == materialPreviewTextures.end())
		return RESOURCE_MANAGER.noTexture;

	return materialPreviewTextures[materialID];
}

void FEEditorPreviewManager::createGameModelPreview(std::string gameModelID)
{
	FEGameModel* gameModel = RESOURCE_MANAGER.getGameModel(gameModelID);

	if (gameModel == nullptr)
		return;

	FEDirectionalLight* currentDirectionalLight = nullptr;
	std::vector<std::string> lightList = SCENE.getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (SCENE.getLight(lightList[i])->getType() == FE_DIRECTIONAL_LIGHT)
		{
			currentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.getLight(lightList[i]));
			break;
		}
	}
	glm::vec3 regularLightRotation = currentDirectionalLight->transform.getRotation();
	currentDirectionalLight->transform.setRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	float regularLightIntensity = currentDirectionalLight->getIntensity();
	currentDirectionalLight->setIntensity(10.0f);

	bool regularFog = RENDERER.isDistanceFogEnabled();
	RENDERER.setDistanceFogEnabled(false);

	previewGameModel->mesh = gameModel->mesh;
	previewGameModel->material = gameModel->material;
	previewEntity->setReceivingShadows(false);

	previewFB->bind();
	// We use this values even with deffered renderer because final image will not be gamma corrected, so values over 1.0f would not work.
	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	FocalEngine::FETransformComponent regularMeshTransform = previewEntity->transform;
	glm::vec3 regularCameraPosition = ENGINE.getCamera()->getPosition();
	float regularAspectRation = ENGINE.getCamera()->getAspectRatio();
	float regularCameraPitch = ENGINE.getCamera()->getPitch();
	float regularCameraRoll = ENGINE.getCamera()->getRoll();
	float regularCameraYaw = ENGINE.getCamera()->getYaw();
	float regularExposure = ENGINE.getCamera()->getExposure();
	ENGINE.getCamera()->setExposure(1.0f);

	// transform has influence on AABB, so before any calculations setting needed values
	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(15.0, -15.0, 0.0));

	FEAABB meshAABB = previewEntity->getAABB();
	glm::vec3 min = meshAABB.getMin();
	glm::vec3 max = meshAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	previewEntity->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	ENGINE.getCamera()->setPosition(glm::vec3(0.0, 0.0, std::max(std::max(xSize, ySize), zSize) * 1.75f));

	ENGINE.getCamera()->setAspectRatio(1.0f);
	FE_GL_ERROR(glViewport(0, 0, 128, 128));

	ENGINE.getCamera()->setPitch(0.0f);
	ENGINE.getCamera()->setRoll(0.0f);
	ENGINE.getCamera()->setYaw(0.0f);

	// rendering game model to texture
#ifdef USE_DEFERRED_RENDERER
	RENDERER.renderEntityForward(previewEntity, ENGINE.getCamera(), true);
#else
	RENDERER.renderEntity(previewEntity, ENGINE.getCamera(), true);
#endif // USE_DEFERRED_RENDERER
	// reversing all of our transformating.
	previewEntity->transform = regularMeshTransform;

	ENGINE.getCamera()->setPosition(regularCameraPosition);
	ENGINE.getCamera()->setAspectRatio(regularAspectRation);
	ENGINE.getCamera()->setPitch(regularCameraPitch);
	ENGINE.getCamera()->setRoll(regularCameraRoll);
	ENGINE.getCamera()->setYaw(regularCameraYaw);
	ENGINE.getCamera()->setExposure(regularExposure);

	currentDirectionalLight->transform.setRotation(regularLightRotation);
	currentDirectionalLight->setIntensity(regularLightIntensity);

	RENDERER.setDistanceFogEnabled(regularFog);

	previewFB->unBind();

	glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w);

	// if we are updating preview we should delete old texture.
	if (gameModelPreviewTextures.find(gameModelID) != gameModelPreviewTextures.end())
		delete gameModelPreviewTextures[gameModelID];

	gameModelPreviewTextures[gameModelID] = previewFB->getColorAttachment();
	previewFB->setColorAttachment(RESOURCE_MANAGER.createSameFormatTexture(previewFB->getColorAttachment()));
}

void FEEditorPreviewManager::createGameModelPreview(FEGameModel* gameModel, FETexture** resultingTexture)
{
	if (gameModel == nullptr)
		return;

	FEDirectionalLight* currentDirectionalLight = nullptr;
	std::vector<std::string> lightList = SCENE.getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (SCENE.getLight(lightList[i])->getType() == FE_DIRECTIONAL_LIGHT)
		{
			currentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(SCENE.getLight(lightList[i]));
			break;
		}
	}
	glm::vec3 regularLightRotation = currentDirectionalLight->transform.getRotation();
	currentDirectionalLight->transform.setRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	float regularLightIntensity = currentDirectionalLight->getIntensity();
	currentDirectionalLight->setIntensity(5.0f);

	previewGameModel->mesh = gameModel->mesh;
	previewGameModel->material = gameModel->material;
	previewEntity->setReceivingShadows(false);

	if (*resultingTexture == nullptr)
		*resultingTexture = RESOURCE_MANAGER.createSameFormatTexture(previewFB->getColorAttachment());
	FETexture* tempTexture = previewFB->getColorAttachment();
	previewFB->setColorAttachment(*resultingTexture);
	previewFB->bind();
	// We use this values even with deffered renderer because final image will not be gamma corrected, so values over 1.0f would not work.
	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	FocalEngine::FETransformComponent regularMeshTransform = previewEntity->transform;
	glm::vec3 regularCameraPosition = ENGINE.getCamera()->getPosition();
	float regularAspectRation = ENGINE.getCamera()->getAspectRatio();
	float regularCameraPitch = ENGINE.getCamera()->getPitch();
	float regularCameraRoll = ENGINE.getCamera()->getRoll();
	float regularCameraYaw = ENGINE.getCamera()->getYaw();
	float regularExposure = ENGINE.getCamera()->getExposure();
	ENGINE.getCamera()->setExposure(1.0f);

	// transform has influence on AABB, so before any calculations setting needed values
	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(15.0, -15.0, 0.0));

	FEAABB meshAABB = previewEntity->getAABB();
	glm::vec3 min = meshAABB.getMin();
	glm::vec3 max = meshAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	previewEntity->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	ENGINE.getCamera()->setPosition(glm::vec3(0.0, 0.0, std::max(std::max(xSize, ySize), zSize) * 1.75f));

	ENGINE.getCamera()->setAspectRatio(1.0f);
	FE_GL_ERROR(glViewport(0, 0, 128, 128));

	ENGINE.getCamera()->setPitch(0.0f);
	ENGINE.getCamera()->setRoll(0.0f);
	ENGINE.getCamera()->setYaw(0.0f);

	// rendering game model to texture
#ifdef USE_DEFERRED_RENDERER
	RENDERER.renderEntityForward(previewEntity, ENGINE.getCamera(), true);
#else
	RENDERER.renderEntity(previewEntity, ENGINE.getCamera(), true);
#endif // USE_DEFERRED_RENDERER

	// reversing all of our transformating.
	previewEntity->transform = regularMeshTransform;

	ENGINE.getCamera()->setPosition(regularCameraPosition);
	ENGINE.getCamera()->setAspectRatio(regularAspectRation);
	ENGINE.getCamera()->setPitch(regularCameraPitch);
	ENGINE.getCamera()->setRoll(regularCameraRoll);
	ENGINE.getCamera()->setYaw(regularCameraYaw);
	ENGINE.getCamera()->setExposure(regularExposure);

	currentDirectionalLight->transform.setRotation(regularLightRotation);
	currentDirectionalLight->setIntensity(regularLightIntensity);

	previewFB->unBind();
	glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w);
	previewFB->setColorAttachment(tempTexture);
}

FETexture* FEEditorPreviewManager::getGameModelPreview(std::string gameModelID)
{
	// if game model's dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.getGameModel(gameModelID)->getDirtyFlag())
	{
		createGameModelPreview(gameModelID);
		RESOURCE_MANAGER.getGameModel(gameModelID)->setDirtyFlag(false);
	}

	// if game model's material dirty flag is set we need to update preview
	if (RESOURCE_MANAGER.getGameModel(gameModelID)->getMaterial() != nullptr && RESOURCE_MANAGER.getGameModel(gameModelID)->getMaterial()->getDirtyFlag())
	{
		createMaterialPreview(RESOURCE_MANAGER.getGameModel(gameModelID)->getMaterial()->getObjectID());
		// This material could use muiltiple GM so we should update all GMs.
		updateAllGameModelPreviews();
		RESOURCE_MANAGER.getGameModel(gameModelID)->getMaterial()->setDirtyFlag(false);
		//createGameModelPreview(gameModelID);
	}

	// if we somehow could not find preview, we will create it.
	if (gameModelPreviewTextures.find(gameModelID) == gameModelPreviewTextures.end())
		createGameModelPreview(gameModelID);

	// if still we don't have it
	if (gameModelPreviewTextures.find(gameModelID) == gameModelPreviewTextures.end())
		return RESOURCE_MANAGER.noTexture;

	return gameModelPreviewTextures[gameModelID];
}

void FEEditorPreviewManager::clear()
{
	auto iterator = meshPreviewTextures.begin();
	while (iterator != meshPreviewTextures.end())
	{
		delete iterator->second;
		iterator++;
	}
	meshPreviewTextures.clear();

	iterator = materialPreviewTextures.begin();
	while (iterator != materialPreviewTextures.end())
	{
		delete iterator->second;
		iterator++;
	}
	materialPreviewTextures.clear();

	iterator = gameModelPreviewTextures.begin();
	while (iterator != gameModelPreviewTextures.end())
	{
		delete iterator->second;
		iterator++;
	}
	gameModelPreviewTextures.clear();
}

void FEEditorPreviewManager::updateAllGameModelPreviews()
{
	// Geting list of all game models.
	auto GMList = RESOURCE_MANAGER.getGameModelList();
	for (size_t i = 0; i < GMList.size(); i++)
	{
		createGameModelPreview(GMList[i]);
	}
}