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

	FEPixelAccurateSelection = RESOURCE_MANAGER.createShader("FEPixelAccurateSelection", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_VS.glsl").c_str(),
																						 RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 "4279660C7D3D27360358354E"/*"FEPixelAccurateSelection"*/);
	RESOURCE_MANAGER.makeShaderStandard(FEPixelAccurateSelection);
	pixelAccurateSelectionMaterial->shader = FEPixelAccurateSelection;

	FEPixelAccurateInstancedSelection = RESOURCE_MANAGER.createShader("FEPixelAccurateInstancedSelection", RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_INSTANCED_VS.glsl").c_str(),
																										   RESOURCE_MANAGER.loadGLSL("Editor//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																									       nullptr,
																										   nullptr,
																										   nullptr,
																										   nullptr,
																										   "0E213D3542135C15471F0D6B"/*"FEPixelAccurateInstancedSelection"*/);

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

FEObject* FEEditorSelectedObject::getSelected()
{
	return container;
}

bool FEEditorSelectedObject::getDirtyFlag()
{
	return dirtyFlag;
}

void FEEditorSelectedObject::setDirtyFlag(bool newValue)
{
	dirtyFlag = newValue;
}

void FEEditorSelectedObject::setSelected(FEObject* selectedObject)
{
	if (selectedObject == nullptr)
		return;
	
	if (container != selectedObject)
		dirtyFlag = true;
	container = selectedObject;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

void FEEditorSelectedObject::clear()
{
	if (instancedSubObjectIndexSelected != -1 && SCENE.getEntityInstanced(container->getObjectID()) != nullptr)
	{
		SCENE.getEntityInstanced(container->getObjectID())->setSelectMode(false);
	}

	instancedSubObjectIndexSelected = -1;
	container = nullptr;
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
	SELECTED.instancedSubObjectsInfo.clear();

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
					instancedSubObjectsInfo[instancedEntity] = std::vector<int>();
					for (size_t j = 0; j < instancedEntity->instancedAABB.size(); j++)
					{
						if(instancedEntity->instancedAABB[j].rayIntersect(ENGINE.getCamera()->getPosition(), mouseRayVector, dis))
						{
							instancedSubObjectsInfo[instancedEntity].push_back(j);
						}
					}
				}

				SELECTED.objectsUnderMouse.push_back(instancedEntity);
			}
			else
			{
				SELECTED.objectsUnderMouse.push_back(SCENE.getEntity(entityList[i]));
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
			SELECTED.objectsUnderMouse.push_back(SCENE.getTerrain(terrainList[i]));
		}
	}
}

int FEEditorSelectedObject::getIndexOfObjectUnderMouse(double mouseX, double mouseY)
{
#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!checkForSelectionisNeeded)
		return -1;
#endif

	checkForSelectionisNeeded = false;

	pixelAccurateSelectionFB->bind();
	FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	for (size_t i = 0; i < SELECTED.objectsUnderMouse.size(); i++)
	{
#ifdef EDITOR_SELECTION_DEBUG_MODE
		int r = (i + 1) * 50 & 255;
		int g = ((i + 1) * 50 >> 8) & 255;
		int b = ((i + 1) * 50 >> 16) & 255;
#else
		int r = (i + 1) & 255;
		int g = ((i + 1) >> 8) & 255;
		int b = ((i + 1) >> 16) & 255;
#endif

		if (SELECTED.objectsUnderMouse[i]->getType() == FE_ENTITY)
		{
			potentiallySelectedEntity = SCENE.getEntity(SELECTED.objectsUnderMouse[i]->getObjectID());
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
		else if (SELECTED.objectsUnderMouse[i]->getType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* potentiallySelectedEntityInstanced = reinterpret_cast<FEEntityInstanced*>(SCENE.getEntity((SELECTED.objectsUnderMouse[i]->getObjectID())));
			if (!potentiallySelectedEntityInstanced->isVisible())
				continue;

			if (instancedSubObjectsInfo.find(potentiallySelectedEntityInstanced) == instancedSubObjectsInfo.end())
			{
				FEMaterial* regularMaterial = potentiallySelectedEntityInstanced->gameModel->material;
				potentiallySelectedEntityInstanced->gameModel->material = pixelAccurateSelectionMaterial;

				pixelAccurateSelectionMaterial->setBaseColor(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
				pixelAccurateSelectionMaterial->clearAllTexturesInfo();
				pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
				pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

				pixelAccurateSelectionMaterial->shader = FEPixelAccurateInstancedSelection;
				FEMaterial* regularBillboardMaterial = potentiallySelectedEntityInstanced->gameModel->getBillboardMaterial();
				potentiallySelectedEntityInstanced->gameModel->setBillboardMaterial(pixelAccurateSelectionMaterial);

				RENDERER.renderEntityInstanced(potentiallySelectedEntityInstanced, ENGINE.getCamera(), nullptr);

				pixelAccurateSelectionMaterial->shader = FEPixelAccurateSelection;
				potentiallySelectedEntityInstanced->gameModel->setBillboardMaterial(regularBillboardMaterial);

				potentiallySelectedEntityInstanced->gameModel->material = regularMaterial;
				pixelAccurateSelectionMaterial->setAlbedoMap(nullptr);
				pixelAccurateSelectionMaterial->setAlbedoMap(nullptr, 1);
			}
		}
		else if (SELECTED.objectsUnderMouse[i]->getType() == FE_TERRAIN)
		{
			FETerrain* potentiallySelectedTerrain = SCENE.getTerrain(SELECTED.objectsUnderMouse[i]->getObjectID());
			if (potentiallySelectedTerrain != nullptr)
			{
				if (!potentiallySelectedTerrain->isVisible())
					continue;

				potentiallySelectedTerrain->shader = FEResourceManager::getInstance().getShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
				potentiallySelectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
				RENDERER.renderTerrain(potentiallySelectedTerrain, ENGINE.getCamera());
				potentiallySelectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f));
				potentiallySelectedTerrain->shader = FEResourceManager::getInstance().getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
			}
		}
	}

	int lastColorShiftIndex = SELECTED.objectsUnderMouse.size() - 1;

	auto it = instancedSubObjectsInfo.begin();
	while (it != instancedSubObjectsInfo.end())
	{
		for (size_t j = 0; j < it->second.size(); j++)
		{
			lastColorShiftIndex++;
			int r = (lastColorShiftIndex + 1) & 255;
			int g = ((lastColorShiftIndex + 1) >> 8) & 255;
			int b = ((lastColorShiftIndex + 1) >> 16) & 255;

			static FEEntity* dummyEntity = new FEEntity(it->first->gameModel, "dummyEntity");
			dummyEntity->gameModel = it->first->gameModel;
			dummyEntity->transform.forceSetTransformMatrix(it->first->getTransformedInstancedMatrix(it->second[j]));

			FEMaterial* regularMaterial = it->first->gameModel->material;
			dummyEntity->gameModel->material = pixelAccurateSelectionMaterial;

			pixelAccurateSelectionMaterial->setBaseColor(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
			pixelAccurateSelectionMaterial->clearAllTexturesInfo();
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
			pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

			RENDERER.renderEntity(dummyEntity, ENGINE.getCamera());

			it->first->gameModel->material = regularMaterial;
		}
		it++;
	}

	FE_GL_ERROR(glReadPixels(GLint(mouseX), GLint(ENGINE.getRenderTargetHeight() - mouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, colorUnderMouse));
	pixelAccurateSelectionFB->unBind();
	FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));

#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (SELECTED.objectsUnderMouse.size() > 0)
	{
		colorIndex = 0;
		colorIndex |= int(colorUnderMouse[2]);
		colorIndex <<= 8;
		colorIndex |= int(colorUnderMouse[1]);
		colorIndex <<= 8;
		colorIndex |= int(colorUnderMouse[0]);

		colorIndex -= 1;

		if (colorIndex != -1 && colorIndex >= (int)SELECTED.objectsUnderMouse.size())
		{
			colorIndex -= SELECTED.objectsUnderMouse.size();

			FEEntityInstanced* selectedSubObjectInInstance = nullptr;
			auto it = instancedSubObjectsInfo.begin();
			while (it != instancedSubObjectsInfo.end())
			{
				if (colorIndex < (int)it->second.size())
				{
					instancedSubObjectIndexSelected = it->second[colorIndex];
					selectedSubObjectInInstance = it->first;
					break;
				}

				colorIndex -= it->second.size();
				it++;
			}

			if (selectedSubObjectInInstance != nullptr)
			{
				for (size_t i = 0; i < SELECTED.objectsUnderMouse.size(); i++)
				{
					if (SELECTED.objectsUnderMouse[i]->getObjectID() == selectedSubObjectInInstance->getObjectID())
					{
						return i;
					}
				}
			}
		}
		else if (colorIndex != -1)
		{
			return colorIndex;
		}
	}

	SELECTED.clear();
	return -1;
#else
	colorIndex = 0;
	colorIndex |= int(colorUnderMouse[2]);
	colorIndex <<= 8;
	colorIndex |= int(colorUnderMouse[1]);
	colorIndex <<= 8;
	colorIndex |= int(colorUnderMouse[0]);

	colorIndex /= 50;
	colorIndex -= 1;

	return -1;
#endif
}

void FEEditorSelectedObject::onCameraUpdate()
{
	HALO_SELECTION_EFFECT.haloObjectsFB->bind();
	HALO_SELECTION_EFFECT.haloMaterial->clearAllTexturesInfo();
	HALO_SELECTION_EFFECT.haloMaterial->setBaseColor(glm::vec3(1.0f, 0.25f, 0.0f));
	FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

	if (container == nullptr)
	{
		HALO_SELECTION_EFFECT.haloObjectsFB->unBind();
		FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));
		HALO_SELECTION_EFFECT.postProcess->active = true;
		return;
	}
	
	if (container->getType() == FE_ENTITY)
	{
		FEEntity* selectedEntity = SCENE.getEntity(container->getObjectID());

		FEMaterial* regularMaterial = selectedEntity->gameModel->material;
		selectedEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

		RENDERER.renderEntity(selectedEntity, ENGINE.getCamera());

		selectedEntity->gameModel->material = regularMaterial;
	}
	else if (container->getType() == FE_ENTITY_INSTANCED)
	{
		FEEntityInstanced* selectedEntity = reinterpret_cast<FEEntityInstanced*>(SCENE.getEntity(container->getObjectID()));

		if (instancedSubObjectIndexSelected != -1)
		{
			static FEEntity* dummyEntity = new FEEntity(selectedEntity->gameModel, "dummyEntity");
			dummyEntity->gameModel = selectedEntity->gameModel;
			dummyEntity->transform.forceSetTransformMatrix(selectedEntity->getTransformedInstancedMatrix(instancedSubObjectIndexSelected));

			FEMaterial* regularMaterial = selectedEntity->gameModel->material;
			dummyEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;

			HALO_SELECTION_EFFECT.haloMaterial->setBaseColor(glm::vec3(0.61f, 0.86f, 1.0f));
			HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
			HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

			RENDERER.renderEntity(dummyEntity, ENGINE.getCamera());

			selectedEntity->gameModel->material = regularMaterial;
		}
		else
		{
			FEMaterial* regularMaterial = selectedEntity->gameModel->material;
			selectedEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;
			HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
			HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

			HALO_SELECTION_EFFECT.haloMaterial->shader = HALO_SELECTION_EFFECT.HaloDrawInstancedObjectShader;
			FEMaterial* regularBillboardMaterial = selectedEntity->gameModel->getBillboardMaterial();
			selectedEntity->gameModel->setBillboardMaterial(HALO_SELECTION_EFFECT.haloMaterial);

			RENDERER.renderEntityInstanced(selectedEntity, ENGINE.getCamera(), nullptr);

			HALO_SELECTION_EFFECT.haloMaterial->shader = HALO_SELECTION_EFFECT.HaloDrawObjectShader;
			selectedEntity->gameModel->setBillboardMaterial(regularBillboardMaterial);

			selectedEntity->gameModel->material = regularMaterial;
		}
		
	}
	else if (container->getType() == FE_TERRAIN)
	{
		FETerrain* selectedTerrain = SCENE.getTerrain(container->getObjectID());

		selectedTerrain->shader = FEResourceManager::getInstance().getShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
		selectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f, 0.25f, 0.0f));
		float regularLODLevel = selectedTerrain->getLODlevel();
		selectedTerrain->setLODlevel(0.0f);
		RENDERER.renderTerrain(selectedTerrain, ENGINE.getCamera());
		selectedTerrain->setLODlevel(regularLODLevel);
		selectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f));
		selectedTerrain->shader = FEResourceManager::getInstance().getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	}

	HALO_SELECTION_EFFECT.haloObjectsFB->unBind();
	FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));
	HALO_SELECTION_EFFECT.postProcess->active = true;
}

int FEEditorSelectedObject::debugGetLastColorIndex()
{
	return colorIndex;
}

void FEEditorSelectedObject::setSelectedByIndex(size_t index)
{
	if (index < 0 || index >= objectsUnderMouse.size())
		return;

	if (container != nullptr)
	{
		if (SCENE.getEntityInstanced(container->getObjectID()) != nullptr)
		{
			if (objectsUnderMouse[index]->getObjectID() != container->getObjectID())
			{
				SCENE.getEntityInstanced(container->getObjectID())->setSelectMode(false);
				instancedSubObjectIndexSelected = -1;
			}
		}
	}

	container = objectsUnderMouse[index];
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

FEEntity* FEEditorSelectedObject::getEntity()
{
	if (container == nullptr)
		return nullptr;


	if (container->getType() == FE_ENTITY || container->getType() == FE_ENTITY_INSTANCED)
	{
		return SCENE.getEntity(container->getObjectID());
	}

	return nullptr;
}

FELight* FEEditorSelectedObject::getLight()
{
	if (container == nullptr)
		return nullptr;

	if (container->getType() == FE_DIRECTIONAL_LIGHT || container->getType() == FE_SPOT_LIGHT || container->getType() == FE_POINT_LIGHT)
	{
		return SCENE.getLight(container->getObjectID());
	}

	return nullptr;
}

FETerrain* FEEditorSelectedObject::getTerrain()
{
	if (container == nullptr)
		return nullptr;

	if (container->getType() == FE_TERRAIN)
	{
		return SCENE.getTerrain(container->getObjectID());
	}

	return nullptr;
}

//template <class T>
//T* FEEditorSelectedObject::getSelected(FEObjectType type)
//{
//	if (container == nullptr)
//		return nullptr;
//
//	if (type == FE_ENTITY || type == FE_ENTITY_INSTANCED)
//	{
//		return SCENE.getEntity(container->getObjectID());
//	}
//
//	return nullptr;
//}