#include "FEEditorSelectedObject.h"
using namespace FocalEngine;

FEEditorSelectedObject* FEEditorSelectedObject::_instance = nullptr;
FEEditorSelectedObject::FEEditorSelectedObject() {}
FEEditorSelectedObject::~FEEditorSelectedObject() {}

void FEEditorSelectedObject::initializeResources()
{
	HALO_SELECTION_EFFECT.initializeResources();

	pixelAccurateSelectionFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.getWindowWidth(), ENGINE.getWindowHeight());
	delete pixelAccurateSelectionFB->getColorAttachment();
	pixelAccurateSelectionFB->setColorAttachment(RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, ENGINE.getWindowWidth(), ENGINE.getWindowHeight()));

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

void FEEditorSelectedObject::setOnUpdateFunc(void(*func)())
{
	onUpdateFunc = func;
}

FEEntity* FEEditorSelectedObject::getEntity()
{
	if (obj == nullptr || (type != FE_ENTITY && type != FE_ENTITY_INSTANCED))
		return nullptr;

	return reinterpret_cast<FEEntity*>(obj);
}

std::string FEEditorSelectedObject::getEntityName()
{
	if (obj == nullptr || (type != FE_ENTITY && type != FE_ENTITY_INSTANCED))
		return "";

	return reinterpret_cast<FEEntity*>(obj)->getName();
}

FETerrain* FEEditorSelectedObject::getTerrain()
{
	if (obj == nullptr || type != FE_TERRAIN)
		return nullptr;

	return reinterpret_cast<FETerrain*>(obj);
}

std::string FEEditorSelectedObject::getTerrainName()
{
	if (obj == nullptr || type != FE_TERRAIN)
		return "";

	return reinterpret_cast<FETerrain*>(obj)->getName();
}

bool FEEditorSelectedObject::isAnyObjectSelected()
{
	return obj != nullptr;
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

	type = selected->getType();
	if (obj != selected)
		dirtyFlag = true;
	obj = selected;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

void FEEditorSelectedObject::setTerrain(FETerrain* selected)
{
	if (selected == nullptr)
		return;

	type = selected->getType();
	if (obj != selected)
		dirtyFlag = true;
	obj = selected;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

void FEEditorSelectedObject::clear()
{
	type = FE_NULL;
	obj = nullptr;
	dirtyFlag = true;
	if (onUpdateFunc != nullptr)
		onUpdateFunc();
}

glm::dvec3 FEEditorSelectedObject::mouseRay(double mouseX, double mouseY)
{
	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / ENGINE.getWindowWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / ENGINE.getWindowHeight();

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
	SELECTED.entitiesUnderMouse.clear();

	glm::vec3 mouseRayVector = mouseRay(mouseX, mouseY);
	std::vector<std::string> entityList = SCENE.getEntityList();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		float dis = 0;
		FEAABB box = SCENE.getEntity(entityList[i])->getAABB();
		if (box.rayIntersect(ENGINE.getCamera()->getPosition(), mouseRayVector, dis))
		{
			SELECTED.entitiesUnderMouse.push_back(entityList[i]);
		}
	}

	std::vector<std::string> terrainList = SCENE.getTerrainList();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		float dis = 0;
		FEAABB box = SCENE.getTerrain(terrainList[i])->getAABB();
		if (box.rayIntersect(ENGINE.getCamera()->getPosition(), mouseRayVector, dis))
		{
			SELECTED.entitiesUnderMouse.push_back(terrainList[i]);
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

	for (size_t i = 0; i < SELECTED.entitiesUnderMouse.size(); i++)
	{
		potentiallySelectedEntity = SCENE.getEntity(SELECTED.entitiesUnderMouse[i]);
		// sometimes we can delete entity before entitiesUnderMouse update
		if (potentiallySelectedEntity == nullptr)
		{
			// but it could be terrain
			FETerrain* potentiallySelectedTerrain = SCENE.getTerrain(SELECTED.entitiesUnderMouse[i]);
			if (potentiallySelectedTerrain != nullptr)
			{
				if (!potentiallySelectedTerrain->isVisible())
					continue;

				int r = (i + 1) & 255;
				int g = ((i + 1) >> 8) & 255;
				int b = ((i + 1) >> 16) & 255;

				potentiallySelectedTerrain->shader = FEResourceManager::getInstance().getShader("FESMTerrainShader");
				potentiallySelectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
				RENDERER.renderTerrain(potentiallySelectedTerrain, ENGINE.getCamera());
				potentiallySelectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f));
				potentiallySelectedTerrain->shader = FEResourceManager::getInstance().getShader("FETerrainShader");
			}
			continue;
		}

		if (!potentiallySelectedEntity->isVisible())
			continue;

		FEMaterial* regularMaterial = potentiallySelectedEntity->gameModel->material;
		potentiallySelectedEntity->gameModel->material = pixelAccurateSelectionMaterial;

		int r = (i + 1) & 255;
		int g = ((i + 1) >> 8) & 255;
		int b = ((i + 1) >> 16) & 255;
		pixelAccurateSelectionMaterial->setBaseColor(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
		pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
		pixelAccurateSelectionMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

		if (potentiallySelectedEntity->getType() == FE_ENTITY_INSTANCED)
		{
			pixelAccurateSelectionMaterial->shader = RESOURCE_MANAGER.getShader("FEPixelAccurateInstancedSelection");
			FEMaterial* regularBillboardMaterial = potentiallySelectedEntity->gameModel->getBillboardMaterial();
			potentiallySelectedEntity->gameModel->setBillboardMaterial(pixelAccurateSelectionMaterial);

			RENDERER.renderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(potentiallySelectedEntity), ENGINE.getCamera(), nullptr);

			pixelAccurateSelectionMaterial->shader = RESOURCE_MANAGER.getShader("FEPixelAccurateSelection");
			potentiallySelectedEntity->gameModel->setBillboardMaterial(regularBillboardMaterial);
		}
		else
		{
			RENDERER.renderEntity(potentiallySelectedEntity, ENGINE.getCamera());
		}

		potentiallySelectedEntity->gameModel->material = regularMaterial;
		pixelAccurateSelectionMaterial->setAlbedoMap(nullptr);
		pixelAccurateSelectionMaterial->setAlbedoMap(nullptr, 1);
	}

	FE_GL_ERROR(glReadPixels(GLint(mouseX), GLint(ENGINE.getWindowHeight() - mouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, colorUnderMouse));
	pixelAccurateSelectionFB->unBind();
	FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));

	if (SELECTED.entitiesUnderMouse.size() > 0)
	{
		colorIndex = 0;
		colorIndex |= int(colorUnderMouse[2]);
		colorIndex <<= 8;
		colorIndex |= int(colorUnderMouse[1]);
		colorIndex <<= 8;
		colorIndex |= int(colorUnderMouse[0]);

		colorIndex -= 1;

		if (SELECTED.entitiesUnderMouse.size() > size_t(colorIndex))
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
	HALO_SELECTION_EFFECT.selectedEntity = SELECTED.getEntity();
	if (HALO_SELECTION_EFFECT.selectedEntity != nullptr)
	{
		HALO_SELECTION_EFFECT.haloObjectsFB->bind();
		FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

		FEMaterial* regularMaterial = HALO_SELECTION_EFFECT.selectedEntity->gameModel->material;
		HALO_SELECTION_EFFECT.selectedEntity->gameModel->material = HALO_SELECTION_EFFECT.haloMaterial;
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap());
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(regularMaterial->getAlbedoMap(1), 1);

		if (HALO_SELECTION_EFFECT.selectedEntity->getType() == FE_ENTITY_INSTANCED)
		{
			HALO_SELECTION_EFFECT.haloMaterial->shader = RESOURCE_MANAGER.getShader("HaloDrawInstancedObjectShader");
			FEMaterial* regularBillboardMaterial = HALO_SELECTION_EFFECT.selectedEntity->gameModel->getBillboardMaterial();
			HALO_SELECTION_EFFECT.selectedEntity->gameModel->setBillboardMaterial(HALO_SELECTION_EFFECT.haloMaterial);

			RENDERER.renderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(HALO_SELECTION_EFFECT.selectedEntity), ENGINE.getCamera(), nullptr/*ENGINE.getCamera()->getFrustumPlanes()*/);

			HALO_SELECTION_EFFECT.haloMaterial->shader = RESOURCE_MANAGER.getShader("HaloDrawObjectShader");
			HALO_SELECTION_EFFECT.selectedEntity->gameModel->setBillboardMaterial(regularBillboardMaterial);
		}
		else
		{
			RENDERER.renderEntity(HALO_SELECTION_EFFECT.selectedEntity, ENGINE.getCamera());
		}

		HALO_SELECTION_EFFECT.selectedEntity->gameModel->material = regularMaterial;
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(nullptr);
		HALO_SELECTION_EFFECT.haloMaterial->setAlbedoMap(nullptr, 1);

		HALO_SELECTION_EFFECT.haloObjectsFB->unBind();
		FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));  
		HALO_SELECTION_EFFECT.postProcess->active = true;
	}
	else if (SELECTED.getTerrain() != nullptr)
	{
		FETerrain* selectedTerrain = SELECTED.getTerrain();

		HALO_SELECTION_EFFECT.haloObjectsFB->bind();
		FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

		selectedTerrain->shader = FEResourceManager::getInstance().getShader("FESMTerrainShader");
		selectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f, 0.25f, 0.0f));
		float regularLODLevel = selectedTerrain->getLODlevel();
		selectedTerrain->setLODlevel(0.0f);
		RENDERER.renderTerrain(selectedTerrain, ENGINE.getCamera());
		selectedTerrain->setLODlevel(regularLODLevel);
		selectedTerrain->shader->getParameter("baseColor")->updateData(glm::vec3(1.0f));
		selectedTerrain->shader = FEResourceManager::getInstance().getShader("FETerrainShader");

		HALO_SELECTION_EFFECT.haloObjectsFB->unBind();
		FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));
		HALO_SELECTION_EFFECT.postProcess->active = true;
	}
	else
	{
		HALO_SELECTION_EFFECT.haloObjectsFB->bind();
		FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

		HALO_SELECTION_EFFECT.haloObjectsFB->unBind();
		FE_GL_ERROR(glClearColor(0.55f, 0.73f, 0.87f, 1.0f));
		HALO_SELECTION_EFFECT.postProcess->active = false;

		return;
	}
}

int FEEditorSelectedObject::debugGetLastColorIndex()
{
	return colorIndex;
}