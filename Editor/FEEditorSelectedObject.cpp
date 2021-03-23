#include "FEEditorSelectedObject.h"
using namespace FocalEngine;

FEEditorSelectedObject* FEEditorSelectedObject::_instance = nullptr;
FEEditorSelectedObject::FEEditorSelectedObject() {}
FEEditorSelectedObject::~FEEditorSelectedObject() {}

void FEEditorSelectedObject::initializeResources()
{
	HALO_SELECTION_EFFECT.initializeResources();

	pixelAccurateSelectionFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight());
	delete pixelAccurateSelectionFB->getColorAttachment();
	pixelAccurateSelectionFB->setColorAttachment(RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight()));

	pixelAccurateSelectionMaterial = RESOURCE_MANAGER.createMaterial("pixelAccurateSelectionMaterial");
	RESOURCE_MANAGER.makeMaterialStandard(pixelAccurateSelectionMaterial);

	pixelAccurateSelectionMaterial->shader = RESOURCE_MANAGER.createShader("FEPixelAccurateSelection", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_VS.glsl").c_str(),
		RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_FS.glsl").c_str());

	RESOURCE_MANAGER.makeShaderStandard(pixelAccurateSelectionMaterial->shader);

	FEShader* FEPixelAccurateInstancedSelection = RESOURCE_MANAGER.createShader("FEPixelAccurateInstancedSelection", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_INSTANCED_VS.glsl").c_str(),
		RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_FS.glsl").c_str());

	RESOURCE_MANAGER.makeShaderStandard(FEPixelAccurateInstancedSelection);

	FEShaderParam colorParam(glm::vec3(0.0f, 0.0f, 0.0f), "baseColor");
	pixelAccurateSelectionMaterial->addParameter(colorParam);
}

void FEEditorSelectedObject::reInitializeResources()
{
	HALO_SELECTION_EFFECT.reInitializeResources();

	delete pixelAccurateSelectionFB;
	pixelAccurateSelectionFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight());
	delete pixelAccurateSelectionFB->getColorAttachment();
	pixelAccurateSelectionFB->setColorAttachment(RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, ENGINE.getRenderTargetWidth(), ENGINE.getRenderTargetHeight()));
}

void FEEditorSelectedObject::setOnUpdateFunc(void(*func)())
{
	onUpdateFunc = func;
}

FEEntity* FEEditorSelectedObject::getEntity()
{
	if (container.objPointer == nullptr || (container.type != SELECTED_ENTITY && container.type != SELECTED_ENTITY_INSTANCED))
		return nullptr;

	return reinterpret_cast<FEEntity*>(container.objPointer);
}

std::string FEEditorSelectedObject::getEntityName()
{
	if (container.objPointer == nullptr || (container.type != SELECTED_ENTITY && container.type != SELECTED_ENTITY_INSTANCED))
		return "";

	return reinterpret_cast<FEEntity*>(container.objPointer)->getName();
}

FETerrain* FEEditorSelectedObject::getTerrain()
{
	if (container.objPointer == nullptr || container.type != SELECTED_TERRAIN)
		return nullptr;

	return reinterpret_cast<FETerrain*>(container.objPointer);
}

std::string FEEditorSelectedObject::getTerrainName()
{
	if (container.objPointer == nullptr || container.type != SELECTED_TERRAIN)
		return "";

	return reinterpret_cast<FETerrain*>(container.objPointer)->getName();
}

bool FEEditorSelectedObject::isAnyObjectSelected()
{
	return container.objPointer != nullptr;
}

bool FEEditorSelectedObject::getDirtyFlag()
{
	return dirtyFlag;
}

void FEEditorSelectedObject::setDirtyFlag(bool newValue)
{
	dirtyFlag = newValue;
}

void FEEditorSelectedObject::setEntity(FEEntity* selected)
{
	if (selected == nullptr)
		return;

	container.type = SELECTED_ENTITY;
	if (container.objPointer != selected)
		dirtyFlag = true;
	container.objPointer = selected;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

void FEEditorSelectedObject::setTerrain(FETerrain* selected)
{
	if (selected == nullptr)
		return;

	container.type = SELECTED_TERRAIN;
	if (container.objPointer != selected)
		dirtyFlag = true;
	container.objPointer = selected;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

void FEEditorSelectedObject::clear()
{
	if (container.type == SELECTED_ENTITY_INSTANCED_SUBOBJECT)
		reinterpret_cast<FEEntityInstanced*>(container.objPointer)->setSelectMode(false);

	container.type = SELECTED_NULL;
	container.objPointer = nullptr;
	dirtyFlag = true;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

glm::dvec3 FEEditorSelectedObject::mouseRay(double mouseX, double mouseY)
{
	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / ENGINE.getRenderTargetWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / ENGINE.getRenderTargetHeight();

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(ENGINE.getCamera()->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(ENGINE.getCamera()->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

void FEEditorSelectedObject::determineEntityUnderMouse(double mouseX, double mouseY)
{
	SELECTED.objectsUnderMouse.clear();

	glm::vec3 mouseRayVector = mouseRay(mouseX, mouseY);
	std::vector<std::string> entityList = SCENE.getEntityList();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		float dis = 0;
		FEAABB box = SCENE.getEntity(entityList[i])->getAABB();
		if (box.rayIntersect(ENGINE.getCamera()->getPosition(), mouseRayVector, dis))
		{
			if (SCENE.getEntity(entityList[i])->getType() == FE_ENTITY_INSTANCED)
			{
				FEEntityInstanced* instancedEntity = reinterpret_cast<FEEntityInstanced*>(SCENE.getEntity(entityList[i]));
				if (instancedEntity->isSelectMode())
				{
					for (size_t j = 0; j < instancedEntity->instancedAABB.size(); j++)
					{
						if (instancedEntity->instancedAABB[j].rayIntersect(ENGINE.getCamera()->getPosition(), mouseRayVector, dis))
						{
							SELECTED.objectsUnderMouse.push_back(selectedObject(SELECTED_ENTITY_INSTANCED_SUBOBJECT, reinterpret_cast<void*>(instancedEntity)));
							SELECTED.objectsUnderMouse.back().additionalInformation = j;
						}
					}
				}
				else
				{
					SELECTED.objectsUnderMouse.push_back(selectedObject(SELECTED_ENTITY_INSTANCED, reinterpret_cast<void*>(instancedEntity)));
				}
			}
			else
			{
				SELECTED.objectsUnderMouse.push_back(selectedObject(SELECTED_ENTITY, reinterpret_cast<void*>(SCENE.getEntity(entityList[i]))));
			}
		}
	}

	std::vector<std::string> terrainList = SCENE.getTerrainList();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		float dis = 0;
		FEAABB box = SCENE.getTerrain(terrainList[i])->getAABB();
		if (box.rayIntersect(ENGINE.getCamera()->getPosition(), mouseRayVector, dis))
		{
			SELECTED.objectsUnderMouse.push_back(selectedObject(SELECTED_TERRAIN, reinterpret_cast<void*>(SCENE.getTerrain(terrainList[i]))));
		}
	}
}

int FEEditorSelectedObject::getIndexOfObjectUnderMouse(double mouseX, double mouseY)
{
	if (!checkForSelectionisNeeded)
		return -1;

	checkForSelectionisNeeded = false;

	pixelAccurateSelectionFB->bind();
	FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	for (size_t i = 0; i < SELECTED.objectsUnderMouse.size(); i++)
	{
		int r = (i + 1) & 255;
		int g = ((i + 1) >> 8) & 255;
		int b = ((i + 1) >> 16) & 255;

		if (SELECTED.objectsUnderMouse[i].type == SELECTED_ENTITY)
		{
			potentiallySelectedEntity = reinterpret_cast<FEEntity*>(SELECTED.objectsUnderMouse[i].objPointer);
			if (!potentiallySelectedEntity->isVisible())
				continue;

			FEMaterial* regularMaterial = potentiallySelectedEntity->gameModel->material;
			potentiallySelectedEntity->gameModel->material = pixelAccurateSelectionMaterial;

			pixelAccurateSelectionMaterial->setBaseColor(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
			pixelAccurateSelectionMaterial->clearAllTexturesInfo();
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

			RENDERER.renderEntity(potentiallySelectedEntity, ENGINE.getCamera());
			
			potentiallySelectedEntity->gameModel->material = regularMaterial;
			pixelAccurateSelectionMaterial->setAlbedoMap(nullptr);
			pixelAccurateSelectionMaterial->setAlbedoMap(nullptr, 1);
		}
		else if (SELECTED.objectsUnderMouse[i].type == SELECTED_ENTITY_INSTANCED)
		{
			FEEntityInstanced* potentiallySelectedEntityInstanced = reinterpret_cast<FEEntityInstanced*>(SELECTED.objectsUnderMouse[i].objPointer);
			if (!potentiallySelectedEntityInstanced->isVisible())
				continue;

			FEMaterial* regularMaterial = potentiallySelectedEntityInstanced->gameModel->material;
			potentiallySelectedEntityInstanced->gameModel->material = pixelAccurateSelectionMaterial;

			pixelAccurateSelectionMaterial->setBaseColor(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
			pixelAccurateSelectionMaterial->clearAllTexturesInfo();
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

			pixelAccurateSelectionMaterial->shader = RESOURCE_MANAGER.getShader("FEPixelAccurateInstancedSelection");
			FEMaterial* regularBillboardMaterial = potentiallySelectedEntityInstanced->gameModel->getBillboardMaterial();
			potentiallySelectedEntityInstanced->gameModel->setBillboardMaterial(pixelAccurateSelectionMaterial);

			RENDERER.renderEntityInstanced(potentiallySelectedEntityInstanced, ENGINE.getCamera(), nullptr);

			pixelAccurateSelectionMaterial->shader = RESOURCE_MANAGER.getShader("FEPixelAccurateSelection");
			potentiallySelectedEntityInstanced->gameModel->setBillboardMaterial(regularBillboardMaterial);

			potentiallySelectedEntityInstanced->gameModel->material = regularMaterial;
			pixelAccurateSelectionMaterial->setAlbedoMap(nullptr);
			pixelAccurateSelectionMaterial->setAlbedoMap(nullptr, 1);
		}
		else if (SELECTED.objectsUnderMouse[i].type == SELECTED_ENTITY_INSTANCED_SUBOBJECT)
		{
			FEEntityInstanced* potentiallySelectedEntityInstanced = reinterpret_cast<FEEntityInstanced*>(SELECTED.objectsUnderMouse[i].objPointer);
			if (!potentiallySelectedEntityInstanced->isVisible())
				continue;

			static FEEntity* dummyEntity = new FEEntity(potentiallySelectedEntityInstanced->gameModel, "dummyEntity");
			dummyEntity->gameModel = potentiallySelectedEntityInstanced->gameModel;
			dummyEntity->transform.forceSetTransformMatrix(potentiallySelectedEntityInstanced->getTransformedInstancedMatrix(SELECTED.objectsUnderMouse[i].additionalInformation));

			//dummyEntity->transform.setPosition(glm::vec3(mat[3][0], mat[3][1], mat[3][2]));
			//dummyEntity->transform.setScale(glm::vec3(glm::length(mat[0])));

			FEMaterial* regularMaterial = potentiallySelectedEntityInstanced->gameModel->material;
			dummyEntity->gameModel->material = pixelAccurateSelectionMaterial;

			pixelAccurateSelectionMaterial->setBaseColor(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
			pixelAccurateSelectionMaterial->clearAllTexturesInfo();
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

			RENDERER.renderEntity(dummyEntity, ENGINE.getCamera());

			potentiallySelectedEntityInstanced->gameModel->material = regularMaterial;
			//pixelAccurateSelectionMaterial->setAlbedoMap(nullptr);
			//pixelAccurateSelectionMaterial->setAlbedoMap(nullptr, 1);
		}
		else if (SELECTED.objectsUnderMouse[i].type == SELECTED_TERRAIN)
		{
			FETerrain* potentiallySelectedTerrain = reinterpret_cast<FETerrain*>(SELECTED.objectsUnderMouse[i].objPointer);
			if (potentiallySelectedTerrain != nullptr)
			{
				if (!potentiallySelectedTerrain->isVisible())
					continue;

				potentiallySelectedTerrain->shader = FEResourceManager::getInstance().getShader("FESMTerrainShader");
				potentiallySelectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
				RENDERER.renderTerrain(potentiallySelectedTerrain, ENGINE.getCamera());
				potentiallySelectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f));
				potentiallySelectedTerrain->shader = FEResourceManager::getInstance().getShader("FETerrainShader");
			}
		}
	}

	FE_GL_ERROR(glReadPixels(GLint(mouseX), GLint(ENGINE.getRenderTargetHeight() - mouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, colorUnderMouse));
	pixelAccurateSelectionFB->unBind();
	FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));

	if (SELECTED.objectsUnderMouse.size() > 0)
	{
		colorIndex = 0;
		colorIndex |= int(colorUnderMouse[2]);
		colorIndex <<= 8;
		colorIndex |= int(colorUnderMouse[1]);
		colorIndex <<= 8;
		colorIndex |= int(colorUnderMouse[0]);

		colorIndex -= 1;

		if (SELECTED.objectsUnderMouse.size() > size_t(colorIndex))
			return colorIndex;

		SELECTED.clear();
	}
	else
	{
		SELECTED.clear();
	}
	
	SELECTED.clear();
	return -1;
}

void FEEditorSelectedObject::onCameraUpdate()
{
	HALO_SELECTION_EFFECT.haloObjectsFB->bind();
	HALO_SELECTION_EFFECT.haloMaterial->clearAllTexturesInfo();
	HALO_SELECTION_EFFECT.haloMaterial->setBaseColor(glm::vec3(1.0f, 0.25f, 0.0f));
	FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

	if (getType() == SELECTED_ENTITY)
	{
		FEEntity* selectedEntity = SELECTED.getEntity();

		FEMaterial* regularMaterial = selectedEntity->gameModel->material;
		selectedEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

		RENDERER.renderEntity(selectedEntity, ENGINE.getCamera());

		selectedEntity->gameModel->material = regularMaterial;
	}
	else if (getType() == SELECTED_ENTITY_INSTANCED)
	{
		FEEntityInstanced* selectedEntity = reinterpret_cast<FEEntityInstanced*>(SELECTED.getEntity());

		FEMaterial* regularMaterial = selectedEntity->gameModel->material;
		selectedEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

		HALO_SELECTION_EFFECT.haloMaterial->shader = RESOURCE_MANAGER.getShader("HaloDrawInstancedObjectShader");
		FEMaterial* regularBillboardMaterial = selectedEntity->gameModel->getBillboardMaterial();
		selectedEntity->gameModel->setBillboardMaterial(HALO_SELECTION_EFFECT.haloMaterial);

		RENDERER.renderEntityInstanced(selectedEntity, ENGINE.getCamera(), nullptr);

		HALO_SELECTION_EFFECT.haloMaterial->shader = RESOURCE_MANAGER.getShader("HaloDrawObjectShader");
		selectedEntity->gameModel->setBillboardMaterial(regularBillboardMaterial);

		selectedEntity->gameModel->material = regularMaterial;
	}
	else if (getType() == SELECTED_ENTITY_INSTANCED_SUBOBJECT)
	{
		FEEntityInstanced* selectedEntity = reinterpret_cast<FEEntityInstanced*>(container.objPointer);

		static FEEntity* dummyEntity = new FEEntity(selectedEntity->gameModel, "dummyEntity");
		dummyEntity->gameModel = selectedEntity->gameModel;
		dummyEntity->transform.forceSetTransformMatrix(selectedEntity->getTransformedInstancedMatrix(container.additionalInformation));

		FEMaterial* regularMaterial = selectedEntity->gameModel->material;
		dummyEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;

		HALO_SELECTION_EFFECT.haloMaterial->setBaseColor(glm::vec3(0.61f, 0.86f, 1.0f));
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

		RENDERER.renderEntity(dummyEntity, ENGINE.getCamera());

		selectedEntity->gameModel->material = regularMaterial;
	}
	else if (getType() == SELECTED_TERRAIN)
	{
		FETerrain* selectedTerrain = SELECTED.getTerrain();

		selectedTerrain->shader = FEResourceManager::getInstance().getShader("FESMTerrainShader");
		selectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f, 0.25f, 0.0f));
		float regularLODLevel = selectedTerrain->getLODlevel();
		selectedTerrain->setLODlevel(0.0f);
		RENDERER.renderTerrain(selectedTerrain, ENGINE.getCamera());
		selectedTerrain->setLODlevel(regularLODLevel);
		selectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f));
		selectedTerrain->shader = FEResourceManager::getInstance().getShader("FETerrainShader");
	}

	HALO_SELECTION_EFFECT.haloObjectsFB->unBind();
	FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));
	HALO_SELECTION_EFFECT.postProcess->active = true;
}

int FEEditorSelectedObject::debugGetLastColorIndex()
{
	return colorIndex;
}

FEEditorSelectedObjectType FEEditorSelectedObject::getType()
{
	return container.type;
}

void FEEditorSelectedObject::setSelectedByIndex(size_t index)
{
	if (index < 0 || index >= objectsUnderMouse.size())
		return;

	container = objectsUnderMouse[index];
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

void* FEEditorSelectedObject::getBareObject()
{
	return container.objPointer;
}

int FEEditorSelectedObject::getAdditionalInformation()
{
	return container.additionalInformation;
}

void FEEditorSelectedObject::setEntity(FEEntityInstanced* selected)
{
	if (selected == nullptr)
		return;

	container.type = SELECTED_ENTITY_INSTANCED;
	if (container.objPointer != selected)
		dirtyFlag = true;
	container.objPointer = selected;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}