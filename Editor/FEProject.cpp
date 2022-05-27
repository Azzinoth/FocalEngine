#include "FEProject.h"

FEProjectManager* FEProjectManager::_instance = nullptr;
FEProjectManager::FEProjectManager() {}
FEProjectManager::~FEProjectManager() {}

void FEProjectManager::initializeResources()
{
	indexChosen = -1;
	loadProjectList();
}

FEProject* FEProjectManager::getCurrent()
{
	return current;
}

void FEProjectManager::setCurrent(FEProject* project)
{
	current = project;
}

std::vector<FEProject*> FEProjectManager::getList()
{
	return list;
}

void FEProjectManager::loadProjectList()
{
	if (!FILE_SYSTEM.isFolder(PROJECTS_FOLDER))
		customProjectFolder = "";

	std::vector<std::string> projectNameList = FILE_SYSTEM.getFolderList(PROJECTS_FOLDER);

	for (size_t i = 0; i < projectNameList.size(); i++)
	{
		if (containProject(std::string(PROJECTS_FOLDER) + std::string("/") + projectNameList[i]))
			list.push_back(new FEProject(projectNameList[i].c_str(), std::string(PROJECTS_FOLDER) + std::string("/") + projectNameList[i].c_str() + "/"));
	}
}

void FEProjectManager::closeCurrentProject()
{
	//closing all windows or popups.
	WindowsManager::getInstance().closeAllWindows();
	WindowsManager::getInstance().closeAllPopups();

	SELECTED.clear();
	for (size_t i = 0; i < list.size(); i++)
	{
		delete list[i];
	}
	list.clear();
	PROJECT_MANAGER.setCurrent(nullptr);
	PREVIEW_MANAGER.clear();

	loadProjectList();

	VIRTUAL_FILE_SYSTEM.setCurrentPath("/");
}

void FEProjectManager::openProject(int projectIndex)
{
	PROJECT_MANAGER.setCurrent(list[projectIndex]);
	PROJECT_MANAGER.getCurrent()->loadScene();
	indexChosen = -1;

	EDITOR_INTERNAL_RESOURCES.internalEditorEntities.clear();
	GIZMO_MANAGER.reInitializeEntities();

	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationYZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateZGizmoEntity);

	// all parts of Gizmos are standard resources except entities, so we need to register them again.
	// if it is first start and those entities are already registered these calls just returns false.
	auto it = EDITOR_INTERNAL_RESOURCES.internalEditorEntities.begin();
	while (it != EDITOR_INTERNAL_RESOURCES.internalEditorEntities.end())
	{
		SCENE.addEntity(it->second);
		it++;
	}

	// after loading project we should update our previews
	PREVIEW_MANAGER.updateAll();
	SELECTED.clear();

	// cleaning dirty flag of entities
	std::vector<std::string> entityList = SCENE.getEntityList();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = SCENE.getEntity(entityList[i]);
		// but before that update AABB
		entity->getAABB();
		entity->transform.setDirtyFlag(false);
	}
}

void FEProjectManager::displayProjectSelection()
{
	static float lowerPanelHight = 90.0f;
	float mainWindowW = float(ENGINE.getWindowWidth());
	float mainWindowH = float(ENGINE.getWindowHeight());

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(mainWindowW, mainWindowH - lowerPanelHight));
	ImGui::Begin("Project Browser", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	{
		ImGui::SetWindowFontScale(2.0f);
		ImGui::Text("CHOOSE WHAT PROJECT TO LOAD :");
		ImGui::SetWindowFontScale(1.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		int columnCount = int(mainWindowW / (512.0f + 32.0f));
		ImGui::Columns(columnCount, "projectColumns", false);
		static bool pushedStyle = false;
		for (size_t i = 0; i < list.size(); i++)
		{
			ImGui::PushID(int(i));
			pushedStyle = false;
			if (indexChosen == i)
			{
				pushedStyle = true;
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
			}

			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (indexChosen != -1)
				{
					openProject(indexChosen);
				}
			}

			if (ImGui::ImageButton((void*)(intptr_t)list[i]->sceneScreenshot->getTextureID(), ImVec2(512.0f, 288.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				indexChosen = int(i);
			}

			ImVec2 textWidth = ImGui::CalcTextSize(list[i]->getName().c_str());
			ImGui::Text(list[i]->getName().c_str());

			if (pushedStyle)
			{
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, mainWindowH - lowerPanelHight));
	ImGui::SetNextWindowSize(ImVec2(mainWindowW, lowerPanelHight));
	ImGui::Begin("##create project", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.0f, 162.0f / 255.0f, 232.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(23.0f / 255.0f, 186.0f / 255.0f, 255.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.0f, 125.0f / 255.0f, 179.0f / 255.0f, 1.0f));

		if (ImGui::Button("Create New Project", ImVec2(200.0f, 64.0f)))
			ImGui::OpenPopup("New Project");

		ImGui::SameLine();
		if (ImGui::Button("Open Project", ImVec2(200.0f, 64.0f)) && indexChosen != -1)
		{
			openProject(indexChosen);
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Project", ImVec2(200.0f, 64.0f)) && indexChosen != -1 && indexChosen < int(list.size()))
		{
			std::string projectFolder = list[indexChosen]->getProjectFolder();
			projectFolder.erase(projectFolder.begin() + projectFolder.size() - 1);

			// Geting list of all files in project folder.
			auto fileList = FILE_SYSTEM.getFileList(list[indexChosen]->getProjectFolder().c_str());
			// We would delete all files in project folder, editor would not create folders there
			// so we are deleting only files.
			for (size_t i = 0; i < fileList.size(); i++)
			{
				FILE_SYSTEM.deleteFile((list[indexChosen]->getProjectFolder() + fileList[i]).c_str());
			}
			// Then we can try to delete project folder, but if user created some folders in it we will fail.
			FILE_SYSTEM.deleteFolder(projectFolder.c_str());

			for (size_t i = 0; i < list.size(); i++)
			{
				delete list[i];
			}
			list.clear();
			PROJECT_MANAGER.setCurrent(nullptr);

			loadProjectList();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.0f, 242.0f / 255.0f, 79.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(34.0f / 255.0f, 255.0f / 255.0f, 106.0f / 255.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.0f, 202.0f / 255.0f, 66.0f / 255.0f, 1.0f));

		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowContentRegionWidth() - 280.0f - 32.0f, ImGui::GetCursorPos().y));
		if (ImGui::Button("Choose projects directory", ImVec2(280.0f, 64.0f)))
		{
			std::string path = "";
			FILE_SYSTEM.showFolderOpenDialog(path);
			setProjectsFolder(path);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
			ImGui::Text("Insert name of new project :");
			static char projectName[512] = "";

			ImGui::InputText("", projectName, IM_ARRAYSIZE(projectName));

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				bool alreadyCreated = false;
				for (size_t i = 0; i < list.size(); i++)
				{
					if (list[i]->getName() == std::string(projectName))
					{
						alreadyCreated = true;
						break;
					}
				}

				if (strlen(projectName) != 0 && !alreadyCreated)
				{
					FILE_SYSTEM.createFolder((std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/").c_str());
					list.push_back(new FEProject(projectName, std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/"));
					list.back()->createDummyScreenshot();
					//SCENE.addLight(FE_DIRECTIONAL_LIGHT, "sun");
					std::fstream file;
					file.open((std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/" + "scene.txt").c_str(), std::ios::out);
					file.write(basicScene, strlen(basicScene));
					file.close();

					//openProject(indexChosen);
					//current->saveScene();

					ImGui::CloseCurrentPopup();
					strcpy_s(projectName, "");
				}
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	ImGui::PopStyleVar();
	ImGui::End();
	ImGui::PopStyleVar();
}

bool FEProjectManager::containProject(std::string path)
{
	if (!FILE_SYSTEM.isFolder(path.c_str()))
		return false;

	if (!FILE_SYSTEM.checkFile((path + "/scene.txt").c_str()))
		return false;

	return true;
}

void FEProjectManager::setProjectsFolder(std::string folderPath)
{
	if (folderPath != "")
	{
		customProjectFolder = folderPath;
		for (size_t i = 0; i < list.size(); i++)
		{
			delete list[i];
		}
		list.clear();
		loadProjectList();
	}
}

FEProject::FEProject(std::string Name, std::string ProjectFolder)
{
	name = Name;
	projectFolder = ProjectFolder;

	std::ifstream screenshotFile((this->getProjectFolder() + "projectScreenShot.texture").c_str());

	if (!screenshotFile.good())
	{
		sceneScreenshot = RESOURCE_MANAGER.noTexture;
	}
	else
	{
		sceneScreenshot = RESOURCE_MANAGER.LoadFETextureUnmanaged((this->getProjectFolder() + "projectScreenShot.texture").c_str());
	}

	screenshotFile.close();
}

FEProject::~FEProject()
{
	if (sceneScreenshot != RESOURCE_MANAGER.noTexture)
		delete sceneScreenshot;
	SCENE.clear();
	RESOURCE_MANAGER.clear();
	ENGINE.resetCamera();
	VIRTUAL_FILE_SYSTEM.clear();
}

std::string FEProject::getName()
{
	return name;
}

void FEProject::setName(std::string newName)
{
	name = newName;
}

std::string FEProject::getProjectFolder()
{
	return projectFolder;
}

void FEProject::writeTransformToJSON(Json::Value& root, FETransformComponent* transform)
{
	root["position"]["X"] = transform->getPosition()[0];
	root["position"]["Y"] = transform->getPosition()[1];
	root["position"]["Z"] = transform->getPosition()[2];
	root["rotation"]["X"] = transform->getRotation()[0];
	root["rotation"]["Y"] = transform->getRotation()[1];
	root["rotation"]["Z"] = transform->getRotation()[2];
	root["scale"]["uniformScaling"] = transform->uniformScaling;
	root["scale"]["X"] = transform->getScale()[0];
	root["scale"]["Y"] = transform->getScale()[1];
	root["scale"]["Z"] = transform->getScale()[2];
}

void FEProject::saveScene(bool fullSave)
{
	Json::Value root;
	std::ofstream sceneFile;

	root["version"] = PROJECTS_FILE_VER;

	if (!fullSave)
	{
		// saving all unSaved objects
		for (size_t i = 0; i < unSavedObjects.size(); i++)
		{
			switch (unSavedObjects[i]->getType())
			{
				case FE_MESH:
				{
					FEMesh* meshToSave = RESOURCE_MANAGER.getMesh(unSavedObjects[i]->getObjectID());
					if (meshToSave != nullptr)
						RESOURCE_MANAGER.saveFEMesh(meshToSave, (getProjectFolder() + meshToSave->getObjectID() + std::string(".model")).c_str());
					break;
				}

				case FE_TEXTURE:
				{
					FETexture* textureToSave = RESOURCE_MANAGER.getTexture(unSavedObjects[i]->getObjectID());
					if (textureToSave != nullptr)
						RESOURCE_MANAGER.saveFETexture(textureToSave, (getProjectFolder() + textureToSave->getObjectID() + std::string(".texture")).c_str());
					break;
				}
			}
		}
	}

	// saving Meshes
	std::vector<std::string> meshList = RESOURCE_MANAGER.getMeshList();
	Json::Value meshData;
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FEMesh* mesh = RESOURCE_MANAGER.getMesh(meshList[i]);
		meshData[mesh->getObjectID()]["ID"] = mesh->getObjectID();
		meshData[mesh->getObjectID()]["name"] = mesh->getName();
		meshData[mesh->getObjectID()]["fileName"] = mesh->getObjectID() + ".model";

		if (fullSave)
			RESOURCE_MANAGER.saveFEMesh(mesh, (getProjectFolder() + mesh->getObjectID() + std::string(".model")).c_str());

		mesh->setDirtyFlag(false);
	}
	root["meshes"] = meshData;

	// saving Textures
	std::vector<std::string> texturesList = RESOURCE_MANAGER.getTextureList();
	Json::Value texturesData;
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		FETexture* texture = RESOURCE_MANAGER.getTexture(texturesList[i]);
		if (!shouldIncludeInSceneFile(texture))
			continue;

		texturesData[texture->getObjectID()]["ID"] = texture->getObjectID();
		texturesData[texture->getObjectID()]["name"] = texture->getName();
		texturesData[texture->getObjectID()]["fileName"] = texture->getObjectID() + ".texture";
		texturesData[texture->getObjectID()]["type"] = texture->getInternalFormat();

		if (fullSave)
			RESOURCE_MANAGER.saveFETexture(texture, (getProjectFolder() + texture->getObjectID() + std::string(".texture")).c_str());

		texture->setDirtyFlag(false);
	}
	root["textures"] = texturesData;

	// saving Materials
	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();
	Json::Value materialData;
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FEMaterial* material = RESOURCE_MANAGER.getMaterial(materialList[i]);

		for (size_t j = 0; j < FE_MAX_TEXTURES_PER_MATERIAL; j++)
		{
			if (material->textures[j] != nullptr)
				materialData[material->getObjectID()]["textures"][std::to_string(j).c_str()] = material->textures[j]->getObjectID();

			if (material->textureBindings[j] != -1)
				materialData[material->getObjectID()]["textureBindings"][std::to_string(j).c_str()] = material->textureBindings[j];

			if (material->textureChannels[j] != -1)
				materialData[material->getObjectID()]["textureChannels"][std::to_string(j).c_str()] = material->textureChannels[j];
		}

		materialData[material->getObjectID()]["ID"] = material->getObjectID();
		materialData[material->getObjectID()]["name"] = material->getName();
		materialData[material->getObjectID()]["metalness"] = material->getMetalness();
		materialData[material->getObjectID()]["roughtness"] = material->getRoughtness();
		materialData[material->getObjectID()]["normalMapIntensity"] = material->getNormalMapIntensity();
		materialData[material->getObjectID()]["ambientOcclusionIntensity"] = material->getAmbientOcclusionIntensity();
		materialData[material->getObjectID()]["ambientOcclusionMapIntensity"] = material->getAmbientOcclusionMapIntensity();
		materialData[material->getObjectID()]["roughtnessMapIntensity"] = material->getRoughtnessMapIntensity();
		materialData[material->getObjectID()]["metalnessMapIntensity"] = material->getMetalnessMapIntensity();
		materialData[material->getObjectID()]["tiling"] = material->getTiling();
		materialData[material->getObjectID()]["compackPacking"] = material->isCompackPacking();

		material->setDirtyFlag(false);
	}
	root["materials"] = materialData;

	// saving gameModels
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	Json::Value gameModelData;
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* gameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
		gameModelData[gameModel->getObjectID()]["ID"] = gameModel->getObjectID();
		gameModelData[gameModel->getObjectID()]["name"] = gameModel->getName();
		gameModelData[gameModel->getObjectID()]["mesh"] = gameModel->mesh->getObjectID();
		gameModelData[gameModel->getObjectID()]["material"] = gameModel->material->getObjectID();
		gameModelData[gameModel->getObjectID()]["scaleFactor"] = gameModel->getScaleFactor();

		gameModelData[gameModel->getObjectID()]["LODs"]["haveLODlevels"] = gameModel->useLODlevels();
		if (gameModel->useLODlevels())
		{
			gameModelData[gameModel->getObjectID()]["LODs"]["cullDistance"] = gameModel->getCullDistance();
			gameModelData[gameModel->getObjectID()]["LODs"]["billboardZeroRotaion"] = gameModel->getBillboardZeroRotaion();
			gameModelData[gameModel->getObjectID()]["LODs"]["LODCount"] = gameModel->getLODCount();
			for (size_t j = 0; j < gameModel->getLODCount(); j++)
			{
				gameModelData[gameModel->getObjectID()]["LODs"][std::to_string(j)]["mesh"] = gameModel->getLODMesh(j)->getObjectID();
				gameModelData[gameModel->getObjectID()]["LODs"][std::to_string(j)]["maxDrawDistance"] = gameModel->getLODMaxDrawDistance(j);
				gameModelData[gameModel->getObjectID()]["LODs"][std::to_string(j)]["isBillboard"] = gameModel->isLODBillboard(j);
				if (gameModel->isLODBillboard(j))
					gameModelData[gameModel->getObjectID()]["LODs"][std::to_string(j)]["billboardMaterial"] = gameModel->getBillboardMaterial()->getObjectID();
			}
		}

		gameModel->setDirtyFlag(false);
	}
	root["gameModels"] = gameModelData;

	// saving prefabs
	std::vector<std::string> prefabList = RESOURCE_MANAGER.getPrefabList();
	Json::Value prefabData;
	for (size_t i = 0; i < prefabList.size(); i++)
	{
		FEPrefab* prefab = RESOURCE_MANAGER.getPrefab(prefabList[i]);
		prefabData[prefab->getObjectID()]["ID"] = prefab->getObjectID();
		prefabData[prefab->getObjectID()]["name"] = prefab->getName();

		Json::Value componentsData;
		for (int j = 0; j < prefab->componentsCount(); j++)
		{
			componentsData[j]["gameModel"]["ID"] = prefab->getComponent(j)->gameModel->getObjectID();
			writeTransformToJSON(componentsData[j]["transformation"], &prefab->getComponent(j)->transform);
		}

		prefabData[prefab->getObjectID()]["components"] = componentsData;
	}
	root["prefabs"] = prefabData;

	// saving Entities
	std::vector<std::string> entityList = SCENE.getEntityList();
	Json::Value entityData;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = SCENE.getEntity(entityList[i]);
		if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(entity))
			continue;

		entityData[entity->getObjectID()]["ID"] = entity->getObjectID();
		entityData[entity->getObjectID()]["type"] = FEObjectTypeToString(entity->getType());
		entityData[entity->getObjectID()]["name"] = entity->getName();
		entityData[entity->getObjectID()]["prefab"] = entity->prefab->getObjectID();
		//entityData[entity->getObjectID()]["gameModel"] = entity->gameModel->getObjectID();
		writeTransformToJSON(entityData[entity->getObjectID()]["transformation"], &entity->transform);

		if (entity->getType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* instancedEntity = reinterpret_cast<FEEntityInstanced*>(entity);
			entityData[entity->getObjectID()]["modificationsToSpawn"] = instancedEntity->getSpawnModificationCount() == 0 ? false : true;
			if (instancedEntity->getSpawnModificationCount())
			{
				std::ofstream infoFile;
				Json::Value entityFileRoot;
				infoFile.open(projectFolder + entity->getObjectID() + ".txt");

				Json::Value modificationsData;
				auto modificationList = instancedEntity->getSpawnModifications();
				for (int j = 0; j < modificationList.size(); j++)
				{
					modificationsData[j]["type"] = int(modificationList[j].type);
					modificationsData[j]["index"] = modificationList[j].index;
					if (modificationList[j].type != CHANGE_DELETED)
					{
						for (int k = 0; k < 4; k++)
						{
							for (int p = 0; p < 4; p++)
							{
								modificationsData[j]["modification"][k][p] = modificationList[j].modification[k][p];
							}
						}
					}
				}
				entityFileRoot["modifications"] = modificationsData;

				infoFile << entityFileRoot;
				infoFile.close();
			}

			entityData[entity->getObjectID()]["spawnInfo"]["seed"] = instancedEntity->spawnInfo.seed;
			entityData[entity->getObjectID()]["spawnInfo"]["objectCount"] = instancedEntity->spawnInfo.objectCount;
			entityData[entity->getObjectID()]["spawnInfo"]["radius"] = instancedEntity->spawnInfo.radius;
			entityData[entity->getObjectID()]["spawnInfo"]["scaleDeviation"] = instancedEntity->spawnInfo.scaleDeviation;
			entityData[entity->getObjectID()]["spawnInfo"]["rotationDeviation.x"] = instancedEntity->spawnInfo.rotationDeviation.x;
			entityData[entity->getObjectID()]["spawnInfo"]["rotationDeviation.y"] = instancedEntity->spawnInfo.rotationDeviation.y;
			entityData[entity->getObjectID()]["spawnInfo"]["rotationDeviation.z"] = instancedEntity->spawnInfo.rotationDeviation.z;
			if (instancedEntity->getSnappedToTerrain() == nullptr)
			{
				entityData[entity->getObjectID()]["snappedToTerrain"] = "none";
			}
			else
			{
				entityData[entity->getObjectID()]["snappedToTerrain"] = instancedEntity->getSnappedToTerrain()->getObjectID();
			}
		}

		entity->setDirtyFlag(false);
	}
	root["entities"] = entityData;

	// saving Terrains
	std::vector<std::string> terrainList = SCENE.getTerrainList();
	Json::Value terrainData;
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* terrain = SCENE.getTerrain(terrainList[i]);

		terrainData[terrain->getObjectID()]["ID"] = terrain->getObjectID();
		terrainData[terrain->getObjectID()]["name"] = terrain->getName();

		terrainData[terrain->getObjectID()]["heightMap"]["ID"] = terrain->heightMap->getObjectID();
		terrainData[terrain->getObjectID()]["heightMap"]["name"] = terrain->heightMap->getName();
		terrainData[terrain->getObjectID()]["heightMap"]["fileName"] = terrain->heightMap->getObjectID() + ".texture";
		RESOURCE_MANAGER.saveFETexture(terrain->heightMap, (getProjectFolder() + terrain->heightMap->getObjectID() + std::string(".texture")).c_str());

		terrainData[terrain->getObjectID()]["hightScale"] = terrain->getHightScale();
		terrainData[terrain->getObjectID()]["displacementScale"] = terrain->getDisplacementScale();
		terrainData[terrain->getObjectID()]["tileMult"]["X"] = terrain->getTileMult().x;
		terrainData[terrain->getObjectID()]["tileMult"]["Y"] = terrain->getTileMult().y;
		terrainData[terrain->getObjectID()]["LODlevel"] = terrain->getLODlevel();
		terrainData[terrain->getObjectID()]["chunkPerSide"] = terrain->getChunkPerSide();

		writeTransformToJSON(terrainData[terrain->getObjectID()]["transformation"], &terrain->transform);

		// Saving terrains Layers.
		for (int i = 0; i < terrain->layerMaps.size(); i++)
		{
			if (terrain->layerMaps[i] != nullptr)
			{
				terrainData[terrain->getObjectID()]["layerMaps"][i]["ID"] = terrain->layerMaps[i]->getObjectID();
				terrainData[terrain->getObjectID()]["layerMaps"][i]["name"] = terrain->layerMaps[i]->getName();
				terrainData[terrain->getObjectID()]["layerMaps"][i]["fileName"] = terrain->layerMaps[i]->getObjectID() + ".texture";
				RESOURCE_MANAGER.saveFETexture(terrain->layerMaps[i], (getProjectFolder() + terrain->layerMaps[i]->getObjectID() + std::string(".texture")).c_str());
			}
		}

		for (int i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
		{
			FETerrainLayer* currentLayer = terrain->getLayerInSlot(i);
			if (currentLayer == nullptr)
			{
				terrainData[terrain->getObjectID()]["layers"][i]["acive"] = false;
				break;
			}

			terrainData[terrain->getObjectID()]["layers"][i]["acive"] = true;
			terrainData[terrain->getObjectID()]["layers"][i]["name"] = currentLayer->getMaterial()->getName();
			terrainData[terrain->getObjectID()]["layers"][i]["materialID"] = currentLayer->getMaterial()->getObjectID();
		}

		terrain->setDirtyFlag(false);
	}
	root["terrains"] = terrainData;

	// saving Lights
	std::vector<std::string> LightList = SCENE.getLightsList();
	Json::Value lightData;
	for (size_t i = 0; i < LightList.size(); i++)
	{
		FELight* light = SCENE.getLight(LightList[i]);

		// general light information
		lightData[light->getObjectID()]["ID"] = light->getObjectID();
		lightData[light->getObjectID()]["type"] = light->getType();
		lightData[light->getObjectID()]["name"] = light->getName();
		lightData[light->getObjectID()]["intensity"] = light->getIntensity();
		writeTransformToJSON(lightData[light->getObjectID()]["transformation"], &light->transform);
		lightData[light->getObjectID()]["castShadows"] = light->isCastShadows();
		lightData[light->getObjectID()]["enabled"] = light->isLightEnabled();
		lightData[light->getObjectID()]["color"]["R"] = light->getColor()[0];
		lightData[light->getObjectID()]["color"]["G"] = light->getColor()[1];
		lightData[light->getObjectID()]["color"]["B"] = light->getColor()[2];
		lightData[light->getObjectID()]["staticShadowBias"] = light->isStaticShadowBias();
		lightData[light->getObjectID()]["shadowBias"] = light->getShadowBias();
		lightData[light->getObjectID()]["shadowBiasVariableIntensity"] = light->getShadowBiasVariableIntensity();
		lightData[light->getObjectID()]["shadowBlurFactor"] = light->getShadowBlurFactor();

		// type specific information
		if (light->getType() == FE_DIRECTIONAL_LIGHT)
		{
			FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);

			lightData[directionalLight->getObjectID()]["direction"]["X"] = directionalLight->getDirection()[0];
			lightData[directionalLight->getObjectID()]["direction"]["Y"] = directionalLight->getDirection()[1];
			lightData[directionalLight->getObjectID()]["direction"]["Z"] = directionalLight->getDirection()[2];
			lightData[directionalLight->getObjectID()]["CSM"]["activeCascades"] = directionalLight->getActiveCascades();
			lightData[directionalLight->getObjectID()]["CSM"]["shadowCoverage"] = directionalLight->getShadowCoverage();
			lightData[directionalLight->getObjectID()]["CSM"]["CSMZDepth"] = directionalLight->getCSMZDepth();
			lightData[directionalLight->getObjectID()]["CSM"]["CSMXYDepth"] = directionalLight->getCSMXYDepth();
		}
		else if (light->getType() == FE_SPOT_LIGHT)
		{
			FESpotLight* spotLight = reinterpret_cast<FESpotLight*>(light);

			lightData[spotLight->getObjectID()]["spotAngle"] = spotLight->getSpotAngle();
			lightData[spotLight->getObjectID()]["spotAngleOuter"] = spotLight->getSpotAngleOuter();
			lightData[spotLight->getObjectID()]["direction"]["X"] = spotLight->getDirection()[0];
			lightData[spotLight->getObjectID()]["direction"]["Y"] = spotLight->getDirection()[1];
			lightData[spotLight->getObjectID()]["direction"]["Z"] = spotLight->getDirection()[2];
		}
		else if (light->getType() == FE_POINT_LIGHT)
		{
			FEPointLight* pointLight = reinterpret_cast<FEPointLight*>(light);

			lightData[pointLight->getObjectID()]["range"] = pointLight->getRange();
		}
	}
	root["lights"] = lightData;

	// saving Effects settings
	Json::Value effectsData;
	// *********** Gamma Correction & Exposure ***********
	effectsData["Gamma Correction & Exposure"]["Gamma"] = ENGINE.getCamera()->getGamma();
	effectsData["Gamma Correction & Exposure"]["Exposure"] = ENGINE.getCamera()->getExposure();
	// *********** Anti-Aliasing(FXAA) ***********
	effectsData["Anti-Aliasing(FXAA)"]["FXAASpanMax"] = RENDERER.getFXAASpanMax();
	effectsData["Anti-Aliasing(FXAA)"]["FXAAReduceMin"] = RENDERER.getFXAAReduceMin();
	effectsData["Anti-Aliasing(FXAA)"]["FXAAReduceMul"] = RENDERER.getFXAAReduceMul();
	// *********** Bloom ***********
	effectsData["Bloom"]["thresholdBrightness"] = RENDERER.getBloomThreshold();
	effectsData["Bloom"]["BloomSize"] = RENDERER.getBloomSize();
	// *********** Depth of Field ***********
	effectsData["Depth of Field"]["Near distance"] = RENDERER.getDOFNearDistance();
	effectsData["Depth of Field"]["Far distance"] = RENDERER.getDOFFarDistance();
	effectsData["Depth of Field"]["Strength"] = RENDERER.getDOFStrength();
	effectsData["Depth of Field"]["Distance dependent strength"] = RENDERER.getDOFDistanceDependentStrength();
	// *********** Distance fog ***********
	effectsData["Distance fog"]["isDistanceFogEnabled"] = RENDERER.isDistanceFogEnabled();
	effectsData["Distance fog"]["Density"] = RENDERER.getDistanceFogDensity();
	effectsData["Distance fog"]["Gradient"] = RENDERER.getDistanceFogGradient();
	// *********** Chromatic Aberration ***********
	effectsData["Chromatic Aberration"]["Shift strength"] = RENDERER.getChromaticAberrationIntensity();
	// *********** Sky ***********
	effectsData["Sky"]["Enabled"] = RENDERER.isSkyEnabled() ? 1.0f : 0.0f;
	effectsData["Sky"]["Sphere size"] = RENDERER.getDistanceToSky();

	root["effects"] = effectsData;

	// saving Camera settings
	Json::Value cameraData;

	cameraData["position"]["X"] = ENGINE.getCamera()->getPosition()[0];
	cameraData["position"]["Y"] = ENGINE.getCamera()->getPosition()[1];
	cameraData["position"]["Z"] = ENGINE.getCamera()->getPosition()[2];

	cameraData["fov"] = ENGINE.getCamera()->getFov();
	cameraData["nearPlane"] = ENGINE.getCamera()->getNearPlane();
	cameraData["farPlane"] = ENGINE.getCamera()->getFarPlane();

	cameraData["yaw"] = ENGINE.getCamera()->getYaw();
	cameraData["pitch"] = ENGINE.getCamera()->getPitch();
	cameraData["roll"] = ENGINE.getCamera()->getRoll();

	cameraData["aspectRatio"] = ENGINE.getCamera()->getAspectRatio();

	cameraData["movementSpeed"] = ENGINE.getCamera()->getMovementSpeed();

	root["camera"] = cameraData;

	// saving into file
	Json::StreamWriterBuilder builder;
	const std::string json_file = Json::writeString(builder, root);

	sceneFile.open(projectFolder + "scene.txt");
	sceneFile << json_file;
	sceneFile.close();

	for (size_t i = 0; i < filesToDelete.size(); i++)
	{
		FILE_SYSTEM.deleteFile(filesToDelete[i].c_str());
	}

	// VFS
	VIRTUAL_FILE_SYSTEM.saveState(projectFolder + "VFS.txt");

	modified = false;
}

void FEProject::readTransformToJSON(Json::Value& root, FETransformComponent* transform)
{
	transform->setPosition(glm::vec3(root["position"]["X"].asFloat(),
		root["position"]["Y"].asFloat(),
		root["position"]["Z"].asFloat()));

	transform->setRotation(glm::vec3(root["rotation"]["X"].asFloat(),
		root["rotation"]["Y"].asFloat(),
		root["rotation"]["Z"].asFloat()));

	transform->uniformScaling = root["scale"]["uniformScaling"].asBool();

	transform->setScale(glm::vec3(root["scale"]["X"].asFloat(),
		root["scale"]["Y"].asFloat(),
		root["scale"]["Z"].asFloat()));
}

void FEProject::loadScene()
{
	std::ifstream sceneFile;
	sceneFile.open(projectFolder + "scene.txt");

	std::string fileData((std::istreambuf_iterator<char>(sceneFile)), std::istreambuf_iterator<char>());
	sceneFile.close();

	Json::Value root;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &root, &err))
		return;

	// read project file version
	float projectVersion = root["version"].asFloat();

	// project file was created before any version was written to project files.
	if (projectVersion != PROJECTS_FILE_VER)
	{
		if (projectVersion == 0.0f)
		{
			LOG.add("Can't find version in scene file of project from " + projectFolder, FE_LOG_WARNING, FE_LOG_LOADING);
			LOG.add("Trying to load project with old version of loader.", FE_LOG_WARNING, FE_LOG_LOADING);
			loadSceneVer0();
			return;
		}
		else if (projectVersion == 0.01f)
		{
			// Do nothing.
		}
	}

	// loading Meshes
	std::vector<Json::String> meshList = root["meshes"].getMemberNames();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		RESOURCE_MANAGER.loadFEMesh((projectFolder + root["meshes"][meshList[i]]["fileName"].asCString()).c_str(), root["meshes"][meshList[i]]["name"].asCString());
	}

	// loading Textures
	std::vector<Json::String> texturesList = root["textures"].getMemberNames();
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		// read type of texture if it is not standard then skip it.
		if (root["textures"][texturesList[i]]["type"] == 33322)
		{
			continue;
		}

		FETexture* loadedTexture = RESOURCE_MANAGER.LoadFETextureAsync((projectFolder + root["textures"][texturesList[i]]["fileName"].asCString()).c_str(), root["textures"][texturesList[i]]["name"].asString(), nullptr, root["textures"][texturesList[i]]["ID"].asString());
	}

	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = RESOURCE_MANAGER.createMaterial(root["materials"][materialsList[i]]["name"].asString(), root["materials"][materialsList[i]]["ID"].asString());

		//newMat->shader = RESOURCE_MANAGER.getShader("FEPhongShader");
		//newMat->shader = RESOURCE_MANAGER.getShader("FESolidColorShader");
		newMat->shader = RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

		std::vector<Json::String> membersList = root["materials"][materialsList[i]].getMemberNames();
		for (size_t j = 0; j < membersList.size(); j++)
		{
			if (membersList[j] == "textures")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textures"].isMember(std::to_string(k).c_str()))
					{
						std::string textureID = root["materials"][materialsList[i]]["textures"][std::to_string(k).c_str()].asCString();
						newMat->textures[k] = RESOURCE_MANAGER.getTexture(textureID);
						if (newMat->textures[k] == nullptr)
							newMat->textures[k] = RESOURCE_MANAGER.noTexture;
					}
				}
			}

			if (membersList[j] == "textureBindings")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textureBindings"].isMember(std::to_string(k).c_str()))
					{
						int binding = root["materials"][materialsList[i]]["textureBindings"][std::to_string(k).c_str()].asInt();
						newMat->textureBindings[k] = binding;
					}
				}
			}

			if (membersList[j] == "textureChannels")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textureChannels"].isMember(std::to_string(k).c_str()))
					{
						int binding = root["materials"][materialsList[i]]["textureChannels"][std::to_string(k).c_str()].asInt();
						newMat->textureChannels[k] = binding;
					}
				}
			}
		}

		newMat->setMetalness(root["materials"][materialsList[i]]["metalness"].asFloat());
		newMat->setRoughtness(root["materials"][materialsList[i]]["roughtness"].asFloat());
		newMat->setNormalMapIntensity(root["materials"][materialsList[i]]["normalMapIntensity"].asFloat());
		newMat->setAmbientOcclusionIntensity(root["materials"][materialsList[i]]["ambientOcclusionIntensity"].asFloat());
		newMat->setAmbientOcclusionMapIntensity(root["materials"][materialsList[i]]["ambientOcclusionMapIntensity"].asFloat());
		newMat->setRoughtnessMapIntensity(root["materials"][materialsList[i]]["roughtnessMapIntensity"].asFloat());
		newMat->setMetalnessMapIntensity(root["materials"][materialsList[i]]["metalnessMapIntensity"].asFloat());

		if (projectVersion >= 0.02f)
		{
			if (root["materials"][materialsList[i]].isMember("tiling"))
				newMat->setTiling(root["materials"][materialsList[i]]["tiling"].asFloat());
			newMat->setCompackPacking(root["materials"][materialsList[i]]["compackPacking"].asBool());
		}
	}

	// loading gameModels
	std::vector<Json::String> gameModelList = root["gameModels"].getMemberNames();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* newGameModel = RESOURCE_MANAGER.createGameModel(RESOURCE_MANAGER.getMesh(root["gameModels"][gameModelList[i]]["mesh"].asCString()),
																	 RESOURCE_MANAGER.getMaterial(root["gameModels"][gameModelList[i]]["material"].asCString()),
																	 root["gameModels"][gameModelList[i]]["name"].asString(), root["gameModels"][gameModelList[i]]["ID"].asString());

		newGameModel->setScaleFactor(root["gameModels"][gameModelList[i]]["scaleFactor"].asFloat());

		bool haveLODLevels = root["gameModels"][gameModelList[i]]["LODs"]["haveLODlevels"].asBool();
		newGameModel->setUsingLODlevels(haveLODLevels);
		if (haveLODLevels)
		{
			newGameModel->setCullDistance(root["gameModels"][gameModelList[i]]["LODs"]["cullDistance"].asFloat());
			newGameModel->setBillboardZeroRotaion(root["gameModels"][gameModelList[i]]["LODs"]["billboardZeroRotaion"].asFloat());

			size_t LODCount = root["gameModels"][gameModelList[i]]["LODs"]["LODCount"].asInt();
			for (size_t j = 0; j < LODCount; j++)
			{
				newGameModel->setLODMesh(j, RESOURCE_MANAGER.getMesh(root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["mesh"].asString()));
				newGameModel->setLODMaxDrawDistance(j, root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["maxDrawDistance"].asFloat());

				bool isLODBillboard = root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["isBillboard"].asBool();
				newGameModel->setIsLODBillboard(j, isLODBillboard);
				if (isLODBillboard)
					newGameModel->setBillboardMaterial(RESOURCE_MANAGER.getMaterial(root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["billboardMaterial"].asString()));
			}
		}
	}

	// loading prefabs
	std::vector<Json::String> prefabList = root["prefabs"].getMemberNames();
	for (size_t i = 0; i < prefabList.size(); i++)
	{
		FEPrefab* newPrefab = RESOURCE_MANAGER.createPrefab(nullptr, root["prefabs"][prefabList[i]]["name"].asString(), root["prefabs"][prefabList[i]]["ID"].asString());

		if (root["prefabs"][prefabList[i]].isMember("components"))
		{
			Json::Value components = root["prefabs"][prefabList[i]]["components"];
			for (int j = 0; j < int(components.size()); j++)
			{
				FETransformComponent componentTransform;
				readTransformToJSON(components[j]["transformation"], &componentTransform);

				newPrefab->addComponent(RESOURCE_MANAGER.getGameModel(components[j]["gameModel"]["ID"].asCString()), componentTransform);
			}
		}
	}

	// loading Terrains
	std::vector<Json::String> terrainList = root["terrains"].getMemberNames();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* newTerrain = RESOURCE_MANAGER.createTerrain(false, root["terrains"][terrainList[i]]["name"].asString(), root["terrains"][terrainList[i]]["ID"].asString());
		newTerrain->heightMap = RESOURCE_MANAGER.LoadFEHeightmap((projectFolder + root["terrains"][terrainList[i]]["heightMap"]["fileName"].asCString()).c_str(), newTerrain, root["terrains"][terrainList[i]]["heightMap"]["name"].asCString());

		newTerrain->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		newTerrain->setDisplacementScale(root["terrains"][terrainList[i]]["displacementScale"].asFloat());
		glm::vec2 tileMult;
		tileMult.x = root["terrains"][terrainList[i]]["tileMult"]["X"].asFloat();
		tileMult.y = root["terrains"][terrainList[i]]["tileMult"]["Y"].asFloat();
		newTerrain->setTileMult(tileMult);
		newTerrain->setLODlevel(root["terrains"][terrainList[i]]["LODlevel"].asFloat());
		newTerrain->setChunkPerSide(root["terrains"][terrainList[i]]["chunkPerSide"].asFloat());
		newTerrain->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		newTerrain->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		readTransformToJSON(root["terrains"][terrainList[i]]["transformation"], &newTerrain->transform);

		if (projectVersion >= 0.02f)
		{
			for (int j = 0; j < FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE; j++)
			{
				if (root["terrains"][terrainList[i]].isMember("layerMaps"))
				{
					FETexture* loadedTexture = RESOURCE_MANAGER.LoadFETextureAsync((projectFolder + root["terrains"][terrainList[i]]["layerMaps"][j]["fileName"].asCString()).c_str(), root["terrains"][terrainList[i]]["layerMaps"][j]["name"].asString(), nullptr, root["terrains"][terrainList[i]]["layerMaps"][j]["ID"].asString());
					newTerrain->layerMaps[j] = loadedTexture;
				}
			}

			for (int j = 0; j < FE_TERRAIN_MAX_LAYERS; j++)
			{
				if (root["terrains"][terrainList[i]]["layers"][j]["acive"].asBool())
				{
					RESOURCE_MANAGER.activateTerrainVacantLayerSlot(newTerrain, RESOURCE_MANAGER.getMaterial(root["terrains"][terrainList[i]]["layers"][j]["materialID"].asCString()));
					newTerrain->getLayerInSlot(j)->setName(root["terrains"][terrainList[i]]["layers"][j]["name"].asCString());
				}
			}
		}

		SCENE.addTerrain(newTerrain);
	}

	// loading Entities
	std::vector<Json::String> entityList = root["entities"].getMemberNames();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		if (root["entities"][entityList[i]].isMember("type"))
		{
			if (root["entities"][entityList[i]]["type"] == "FE_ENTITY_INSTANCED")
			{
				FEEntityInstanced* instancedEntity = nullptr;
				instancedEntity = SCENE.addEntityInstanced(RESOURCE_MANAGER.getPrefab(root["entities"][entityList[i]]["prefab"].asCString()),
																					  root["entities"][entityList[i]]["name"].asString(),
																					  root["entities"][entityList[i]]["ID"].asString());

				readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);

				instancedEntity->spawnInfo.seed = root["entities"][entityList[i]]["spawnInfo"]["seed"].asInt();
				instancedEntity->spawnInfo.objectCount = root["entities"][entityList[i]]["spawnInfo"]["objectCount"].asInt();
				instancedEntity->spawnInfo.radius = root["entities"][entityList[i]]["spawnInfo"]["radius"].asFloat();
				instancedEntity->spawnInfo.scaleDeviation = root["entities"][entityList[i]]["spawnInfo"]["scaleDeviation"].asFloat();
				instancedEntity->spawnInfo.rotationDeviation.x = root["entities"][entityList[i]]["spawnInfo"]["rotationDeviation.x"].asFloat();
				instancedEntity->spawnInfo.rotationDeviation.y = root["entities"][entityList[i]]["spawnInfo"]["rotationDeviation.y"].asFloat();
				instancedEntity->spawnInfo.rotationDeviation.z = root["entities"][entityList[i]]["spawnInfo"]["rotationDeviation.z"].asFloat();

				if (root["entities"][entityList[i]]["snappedToTerrain"].asString() != "none")
				{
					FETerrain* terrain = SCENE.getTerrain(root["entities"][entityList[i]]["snappedToTerrain"].asString());
					terrain->snapInstancedEntity(instancedEntity);
				}

				instancedEntity->populate(instancedEntity->spawnInfo);

				if (root["entities"][entityList[i]]["modificationsToSpawn"].asBool())
				{
					std::ifstream infoFile;
					infoFile.open(projectFolder + instancedEntity->getObjectID() + ".txt");

					std::string infoFileData((std::istreambuf_iterator<char>(infoFile)), std::istreambuf_iterator<char>());

					Json::Value entityFileRoot;
					JSONCPP_STRING err;
					Json::CharReaderBuilder builder;

					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					if (!reader->parse(infoFileData.c_str(), infoFileData.c_str() + infoFileData.size(), &entityFileRoot, &err))
						return;

					size_t count = entityFileRoot["modifications"].size();
					for (int j = 0; j < count; j++)
					{
						if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_DELETED)
						{
							instancedEntity->deleteInstance(entityFileRoot["modifications"][j]["index"].asInt());
						}
						else if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_MODIFIED)
						{
							glm::mat4 modifedMatrix;
							for (int k = 0; k < 4; k++)
							{
								for (int p = 0; p < 4; p++)
								{
									modifedMatrix[k][p] = entityFileRoot["modifications"][j]["modification"][k][p].asFloat();
								}
							}

							instancedEntity->modifyInstance(entityFileRoot["modifications"][int(j)]["index"].asInt(), modifedMatrix);
						}
						else if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_ADDED)
						{
							glm::mat4 modifedMatrix;
							for (int k = 0; k < 4; k++)
							{
								for (int p = 0; p < 4; p++)
								{
									modifedMatrix[k][p] = entityFileRoot["modifications"][j]["modification"][k][p].asFloat();
								}
							}

							instancedEntity->addInstance(modifedMatrix);
						}
					}
				}
			}
			else
			{
				if (root["entities"][entityList[i]].isMember("gameModel"))
				{
					FEPrefab* tempPrefab = RESOURCE_MANAGER.createPrefab(RESOURCE_MANAGER.getGameModel(root["entities"][entityList[i]]["gameModel"].asCString()));
					SCENE.addEntity(tempPrefab, root["entities"][entityList[i]]["name"].asString(), root["entities"][entityList[i]]["ID"].asString());
				}
				else
				{
					SCENE.addEntity(RESOURCE_MANAGER.getPrefab(root["entities"][entityList[i]]["prefab"].asCString()),
									root["entities"][entityList[i]]["name"].asString(),
									root["entities"][entityList[i]]["ID"].asString());
				}
				
				readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);
			}
		}
		else
		{
			SCENE.addEntity(RESOURCE_MANAGER.getPrefab(root["entities"][entityList[i]]["prefab"].asCString()),
							entityList[i],
							root["entities"][entityList[i]]["ID"].asString());
			readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);
		}
	}

	// loading Lights
	std::vector<Json::String> lightList = root["lights"].getMemberNames();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		SCENE.addLight(static_cast<FEObjectType>(root["lights"][lightList[i]]["type"].asInt()), root["lights"][lightList[i]]["name"].asCString(), root["lights"][lightList[i]]["ID"].asCString());
		FELight* light = SCENE.getLight(lightList[i]);

		// general light information
		light->setIntensity(root["lights"][lightList[i]]["intensity"].asFloat());
		readTransformToJSON(root["lights"][lightList[i]]["transformation"], &light->transform);
		light->setCastShadows(root["lights"][lightList[i]]["castShadows"].asBool());
		light->setLightEnabled(root["lights"][lightList[i]]["enabled"].asBool());
		light->setColor(glm::vec3(root["lights"][lightList[i]]["color"]["R"].asFloat(),
								  root["lights"][lightList[i]]["color"]["G"].asFloat(),
								  root["lights"][lightList[i]]["color"]["B"].asFloat()));
		light->setIsStaticShadowBias(root["lights"][lightList[i]]["staticShadowBias"].asBool());
		light->setShadowBias(root["lights"][lightList[i]]["shadowBias"].asFloat());
		light->setShadowBiasVariableIntensity(root["lights"][lightList[i]]["shadowBiasVariableIntensity"].asFloat());
		if (projectVersion >= 0.02f && root["lights"][lightList[i]].isMember("shadowBlurFactor"))
			light->setShadowBlurFactor(root["lights"][lightList[i]]["shadowBlurFactor"].asFloat());

		if (light->getType() == FE_POINT_LIGHT)
		{
			reinterpret_cast<FEPointLight*>(light)->setRange(root["lights"][lightList[i]]["range"].asFloat());
		}
		else if (light->getType() == FE_SPOT_LIGHT)
		{
			reinterpret_cast<FESpotLight*>(light)->setSpotAngle(root["lights"][lightList[i]]["spotAngle"].asFloat());
			reinterpret_cast<FESpotLight*>(light)->setSpotAngleOuter(root["lights"][lightList[i]]["spotAngleOuter"].asFloat());

			reinterpret_cast<FESpotLight*>(light)->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Z"].asFloat()));
		}
		else if (light->getType() == FE_DIRECTIONAL_LIGHT)
		{
			FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);

			directionalLight->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Z"].asFloat()));

			directionalLight->setActiveCascades(root["lights"][lightList[i]]["CSM"]["activeCascades"].asInt());
			directionalLight->setShadowCoverage(root["lights"][lightList[i]]["CSM"]["shadowCoverage"].asFloat());
			directionalLight->setCSMZDepth(root["lights"][lightList[i]]["CSM"]["CSMZDepth"].asFloat());
			directionalLight->setCSMXYDepth(root["lights"][lightList[i]]["CSM"]["CSMXYDepth"].asFloat());
		}
	}

	// loading Effects settings
	// *********** Gamma Correction & Exposure ***********
	ENGINE.getCamera()->setGamma(root["effects"]["Gamma Correction & Exposure"]["Gamma"].asFloat());
	ENGINE.getCamera()->setExposure(root["effects"]["Gamma Correction & Exposure"]["Exposure"].asFloat());
	// *********** Anti-Aliasing(FXAA) ***********
	RENDERER.setFXAASpanMax(root["effects"]["Anti-Aliasing(FXAA)"]["FXAASpanMax"].asFloat());
	RENDERER.setFXAAReduceMin(root["effects"]["Anti-Aliasing(FXAA)"]["FXAAReduceMin"].asFloat());
	RENDERER.setFXAAReduceMul(root["effects"]["Anti-Aliasing(FXAA)"]["FXAAReduceMul"].asFloat());
	// *********** Bloom ***********
	//PPEffect = RENDERER.getPostProcessEffect("bloom");
	RENDERER.setBloomThreshold(root["effects"]["Bloom"]["thresholdBrightness"].asFloat());
	RENDERER.setBloomSize(root["effects"]["Bloom"]["BloomSize"].asFloat());
	// *********** Depth of Field ***********
	RENDERER.setDOFNearDistance(root["effects"]["Depth of Field"]["Near distance"].asFloat());
	RENDERER.setDOFFarDistance(root["effects"]["Depth of Field"]["Far distance"].asFloat());
	RENDERER.setDOFStrength(root["effects"]["Depth of Field"]["Strength"].asFloat());
	RENDERER.setDOFDistanceDependentStrength(root["effects"]["Depth of Field"]["Distance dependent strength"].asFloat());
	// *********** Distance fog ***********
	if (root["effects"]["Distance fog"].isMember("isDistanceFogEnabled"))
	{
		RENDERER.setDistanceFogEnabled(root["effects"]["Distance fog"]["isDistanceFogEnabled"].asBool());
	}
	else
	{
		RENDERER.setDistanceFogEnabled(root["effects"]["Distance fog"]["Density"].asFloat() > -1.0f ? true : false);
	}
	RENDERER.setDistanceFogDensity(root["effects"]["Distance fog"]["Density"].asFloat());
	RENDERER.setDistanceFogGradient(root["effects"]["Distance fog"]["Gradient"].asFloat());
	// *********** Chromatic Aberration ***********
	RENDERER.setChromaticAberrationIntensity(root["effects"]["Chromatic Aberration"]["Shift strength"].asFloat());
	// *********** Sky ***********
	RENDERER.setSkyEnabld(root["effects"]["Sky"]["Enabled"].asFloat() > 0.0f ? true : false);
	RENDERER.setDistanceToSky(root["effects"]["Sky"]["Sphere size"].asFloat());

	// loading Camera settings
	ENGINE.getCamera()->setPosition(glm::vec3(root["camera"]["position"]["X"].asFloat(),
		root["camera"]["position"]["Y"].asFloat(),
		root["camera"]["position"]["Z"].asFloat()));

	ENGINE.getCamera()->setFov(root["camera"]["fov"].asFloat());
	ENGINE.getCamera()->setNearPlane(root["camera"]["nearPlane"].asFloat());
	ENGINE.getCamera()->setFarPlane(root["camera"]["farPlane"].asFloat());

	ENGINE.getCamera()->setYaw(root["camera"]["yaw"].asFloat());
	ENGINE.getCamera()->setPitch(root["camera"]["pitch"].asFloat());
	ENGINE.getCamera()->setRoll(root["camera"]["roll"].asFloat());

	ENGINE.getCamera()->setAspectRatio(root["camera"]["aspectRatio"].asFloat());

	if (projectVersion >= 0.02f && root["camera"].isMember("movementSpeed"))
		ENGINE.getCamera()->setMovementSpeed(root["camera"]["movementSpeed"].asFloat());

	// VFS
	if (FILE_SYSTEM.checkFile((projectFolder + "VFS.txt").c_str()))
	{
		VIRTUAL_FILE_SYSTEM.loadState(projectFolder + "VFS.txt");

		VIRTUAL_FILE_SYSTEM.setDirectoryReadOnly(false, "/Shaders");

		auto files = VIRTUAL_FILE_SYSTEM.getDirectoryContent("/Shaders");
		for (size_t i = 0; i < files.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.deleteFile(files[i], "/Shaders");
		}

		std::vector<std::string> shaderList = RESOURCE_MANAGER.getShadersList();
		for (size_t i = 0; i < shaderList.size(); i++)
		{
			if (FEObjectManager::getInstance().getFEObject(shaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(shaderList[i]), "/Shaders");
		}

		std::vector<std::string> standardShaderList = RESOURCE_MANAGER.getStandardShadersList();
		for (size_t i = 0; i < standardShaderList.size(); i++)
		{
			if (FEObjectManager::getInstance().getFEObject(standardShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(standardShaderList[i]), "/Shaders");
		}

		VIRTUAL_FILE_SYSTEM.setDirectoryReadOnly(true, "/Shaders");
	}
	else
	{
		LOG.add("Can't find VIRTUAL_FILE_SYSTEM file in project folder. Creating basic VIRTUAL_FILE_SYSTEM layout.", FE_LOG_WARNING, FE_LOG_LOADING);
		VIRTUAL_FILE_SYSTEM.createDirectory("Shaders", "/");

		std::vector<std::string> shaderList = RESOURCE_MANAGER.getShadersList();
		for (size_t i = 0; i < shaderList.size(); i++)
		{
			if (FEObjectManager::getInstance().getFEObject(shaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(shaderList[i]), "/Shaders");
		}

		std::vector<std::string> standardShaderList = RESOURCE_MANAGER.getStandardShadersList();
		for (size_t i = 0; i < standardShaderList.size(); i++)
		{
			if (FEObjectManager::getInstance().getFEObject(standardShaderList[i]) == nullptr)
				continue;
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(standardShaderList[i]), "/Shaders");
		}

		std::vector<std::string> meshList_ = RESOURCE_MANAGER.getMeshList();
		for (size_t i = 0; i < meshList_.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(meshList_[i]), "/");
		}

		std::vector<std::string> textureList = RESOURCE_MANAGER.getTextureList();
		for (size_t i = 0; i < textureList.size(); i++)
		{
			bool shouldAdd = true;
			FETexture* textureToAdd = RESOURCE_MANAGER.getTexture(textureList[i]);
			//if (textureToAdd->getInternalFormat() == GL_R16)
			//{
			//	// Potentially it could be texture hight map.
			//	std::vector<std::string> terrainList = SCENE.getTerrainList();
			//	for (size_t j = 0; j < terrainList.size(); j++)
			//	{
			//		if (SCENE.getTerrain(terrainList[j])->heightMap == textureToAdd)
			//		{
			//			shouldAdd = false;
			//			break;
			//		}
			//	}
			//}

			if (shouldAdd)
				VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(textureList[i]), "/");
		}

		std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();
		for (size_t i = 0; i < materialList.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(materialList[i]), "/");
		}

		std::vector<std::string> gameModelList_ = RESOURCE_MANAGER.getGameModelList();
		for (size_t i = 0; i < gameModelList_.size(); i++)
		{
			VIRTUAL_FILE_SYSTEM.createFile(FEObjectManager::getInstance().getFEObject(gameModelList_[i]), "/");
		}
	}
}

void FEProject::createDummyScreenshot()
{
	size_t width = ENGINE.getRenderTargetWidth();
	size_t height = ENGINE.getRenderTargetHeight();

	unsigned char* pixels = new unsigned char[4 * width * height];
	for (size_t j = 0; j < height; j++)
	{
		for (size_t i = 0; i < 4 * width; i += 4)
		{
			pixels[i + (j * 4 * width)] = 0;
			pixels[i + 1 + (j * 4 * width)] = (char)162;
			pixels[i + 2 + (j * 4 * width)] = (char)232;
			pixels[i + 3 + (j * 4 * width)] = (char)255;
		}
	}

	FETexture* tempTexture = RESOURCE_MANAGER.rawDataToFETexture(pixels, int(width), int(height));
	RESOURCE_MANAGER.saveFETexture(tempTexture, (getProjectFolder() + "/projectScreenShot.texture").c_str());
	RESOURCE_MANAGER.deleteFETexture(tempTexture);
	delete[] pixels;
}

void FEProject::addFileToDeleteList(std::string fileName)
{
	filesToDelete.push_back(fileName);
}

void FEProject::loadSceneVer0()
{
	std::ifstream sceneFile;
	sceneFile.open(projectFolder + "scene.txt");

	std::string fileData((std::istreambuf_iterator<char>(sceneFile)), std::istreambuf_iterator<char>());

	Json::Value root;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &root, &err))
		return;

	// correction for loading Meshes
	std::unordered_map<std::string, std::string> meshNameToID;
	meshNameToID["sphere"] = RESOURCE_MANAGER.getMesh("7F251E3E0D08013E3579315F")->getObjectID();
	meshNameToID["cube"] = RESOURCE_MANAGER.getMesh("84251E6E0D0801363579317R")->getObjectID();
	meshNameToID["plane"] = RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156")->getObjectID();
	// loading Meshes
	std::vector<Json::String> meshList = root["meshes"].getMemberNames();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		RESOURCE_MANAGER.loadFEMesh((projectFolder + root["meshes"][meshList[i]]["fileName"].asCString()).c_str(), root["meshes"][meshList[i]]["name"].asCString());
		meshNameToID[root["meshes"][meshList[i]]["name"].asCString()] = root["meshes"][meshList[i]]["ID"].asCString();
	}
	// correction for loading Textures
	std::unordered_map<std::string, std::string> textureNameToID;
	// loading Textures
	std::vector<Json::String> texturesList = root["textures"].getMemberNames();
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		// read type of texture if it is not standard then skip it.
		if (root["textures"][texturesList[i]]["type"] == 33322)
		{
			continue;
		}

		FETexture* loadedTexture = RESOURCE_MANAGER.LoadFETextureAsync((projectFolder + root["textures"][texturesList[i]]["fileName"].asCString()).c_str(), root["textures"][texturesList[i]]["name"].asString());
		textureNameToID[root["textures"][texturesList[i]]["name"].asString()] = loadedTexture->getObjectID();
	}

	// correction for loading Materials
	std::unordered_map<std::string, std::string> materialNameToID;
	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = RESOURCE_MANAGER.createMaterial(materialsList[i], root["materials"][materialsList[i]]["ID"].asString());
		materialNameToID[materialsList[i]] = newMat->getObjectID();

		//newMat->shader = RESOURCE_MANAGER.getShader("FEPhongShader");
		//newMat->shader = RESOURCE_MANAGER.getShader("FESolidColorShader");
		newMat->shader = RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

		std::vector<Json::String> membersList = root["materials"][materialsList[i]].getMemberNames();
		for (size_t j = 0; j < membersList.size(); j++)
		{
			if (membersList[j] == "textures")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textures"].isMember(std::to_string(k).c_str()))
					{
						std::string textureID = textureNameToID[root["materials"][materialsList[i]]["textures"][std::to_string(k).c_str()].asCString()];
						newMat->textures[k] = RESOURCE_MANAGER.getTexture(textureID);
					}
				}
			}

			if (membersList[j] == "textureBindings")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textureBindings"].isMember(std::to_string(k).c_str()))
					{
						int binding = root["materials"][materialsList[i]]["textureBindings"][std::to_string(k).c_str()].asInt();
						newMat->textureBindings[k] = binding;
					}
				}
			}

			if (membersList[j] == "textureChannels")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textureChannels"].isMember(std::to_string(k).c_str()))
					{
						int binding = root["materials"][materialsList[i]]["textureChannels"][std::to_string(k).c_str()].asInt();
						newMat->textureChannels[k] = binding;
					}
				}
			}
		}

		newMat->setMetalness(root["materials"][materialsList[i]]["metalness"].asFloat());
		newMat->setRoughtness(root["materials"][materialsList[i]]["roughtness"].asFloat());
		newMat->setNormalMapIntensity(root["materials"][materialsList[i]]["normalMapIntensity"].asFloat());
		newMat->setAmbientOcclusionIntensity(root["materials"][materialsList[i]]["ambientOcclusionIntensity"].asFloat());
		newMat->setAmbientOcclusionMapIntensity(root["materials"][materialsList[i]]["ambientOcclusionMapIntensity"].asFloat());
		newMat->setRoughtnessMapIntensity(root["materials"][materialsList[i]]["roughtnessMapIntensity"].asFloat());
		newMat->setMetalnessMapIntensity(root["materials"][materialsList[i]]["metalnessMapIntensity"].asFloat());
	}

	// correction for loading gameModels
	std::unordered_map<std::string, std::string> gameModelNameToID;
	// loading gameModels
	std::vector<Json::String> gameModelList = root["gameModels"].getMemberNames();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* newGameModel = RESOURCE_MANAGER.createGameModel(RESOURCE_MANAGER.getMesh(meshNameToID[root["gameModels"][gameModelList[i]]["mesh"].asCString()]),
			RESOURCE_MANAGER.getMaterial(materialNameToID[root["gameModels"][gameModelList[i]]["material"].asCString()]),
			gameModelList[i], root["gameModels"][gameModelList[i]]["ID"].asString());
		gameModelNameToID[gameModelList[i]] = root["gameModels"][gameModelList[i]]["ID"].asString();

		newGameModel->setScaleFactor(root["gameModels"][gameModelList[i]]["scaleFactor"].asFloat());

		bool haveLODLevels = root["gameModels"][gameModelList[i]]["LODs"]["haveLODlevels"].asBool();
		newGameModel->setUsingLODlevels(haveLODLevels);
		if (haveLODLevels)
		{
			newGameModel->setCullDistance(root["gameModels"][gameModelList[i]]["LODs"]["cullDistance"].asFloat());
			newGameModel->setBillboardZeroRotaion(root["gameModels"][gameModelList[i]]["LODs"]["billboardZeroRotaion"].asFloat());

			size_t LODCount = root["gameModels"][gameModelList[i]]["LODs"]["LODCount"].asInt();
			for (size_t j = 0; j < LODCount; j++)
			{
				newGameModel->setLODMesh(j, RESOURCE_MANAGER.getMesh(meshNameToID[root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["mesh"].asString()]));
				newGameModel->setLODMaxDrawDistance(j, root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["maxDrawDistance"].asFloat());

				bool isLODBillboard = root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["isBillboard"].asBool();
				newGameModel->setIsLODBillboard(j, isLODBillboard);
				if (isLODBillboard)
					newGameModel->setBillboardMaterial(RESOURCE_MANAGER.getMaterial(materialNameToID[root["gameModels"][gameModelList[i]]["LODs"][std::to_string(j)]["billboardMaterial"].asString()]));
			}
		}
	}

	// correction for loading Terrains
	std::unordered_map<std::string, std::string> terrainNameToID;
	// loading Terrains
	std::vector<Json::String> terrainList = root["terrains"].getMemberNames();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* newTerrain = RESOURCE_MANAGER.createTerrain(false, terrainList[i], root["terrains"][terrainList[i]]["ID"].asString());
		terrainNameToID[newTerrain->getName()] = newTerrain->getObjectID();
		newTerrain->heightMap = RESOURCE_MANAGER.LoadFEHeightmap((projectFolder + root["terrains"][terrainList[i]]["heightMap"]["fileName"].asCString()).c_str(), newTerrain, root["terrains"][terrainList[i]]["heightMap"]["name"].asCString());

		newTerrain->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		newTerrain->setDisplacementScale(root["terrains"][terrainList[i]]["displacementScale"].asFloat());
		glm::vec2 tileMult;
		tileMult.x = root["terrains"][terrainList[i]]["tileMult"]["X"].asFloat();
		tileMult.y = root["terrains"][terrainList[i]]["tileMult"]["Y"].asFloat();
		newTerrain->setTileMult(tileMult);
		newTerrain->setLODlevel(root["terrains"][terrainList[i]]["LODlevel"].asFloat());
		newTerrain->setChunkPerSide(root["terrains"][terrainList[i]]["chunkPerSide"].asFloat());
		newTerrain->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		newTerrain->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		readTransformToJSON(root["terrains"][terrainList[i]]["transformation"], &newTerrain->transform);

		SCENE.addTerrain(newTerrain);
	}

	// loading Entities
	std::vector<Json::String> entityList = root["entities"].getMemberNames();
	std::string entityID;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		if (root["entities"][entityList[i]].isMember("type"))
		{
			if (root["entities"][entityList[i]]["type"] == "FE_ENTITY_INSTANCED")
			{
				entityID = root["entities"][entityList[i]]["ID"].asString();
				FEEntityInstanced* instancedEntity = SCENE.addEntityInstanced(RESOURCE_MANAGER.getGameModel(gameModelNameToID[root["entities"][entityList[i]]["gameModel"].asCString()]), entityList[i], entityID);
				readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityID)->transform);

				instancedEntity->spawnInfo.seed = root["entities"][entityList[i]]["spawnInfo"]["seed"].asInt();
				instancedEntity->spawnInfo.objectCount = root["entities"][entityList[i]]["spawnInfo"]["objectCount"].asInt();
				instancedEntity->spawnInfo.radius = root["entities"][entityList[i]]["spawnInfo"]["radius"].asFloat();
				instancedEntity->spawnInfo.scaleDeviation = root["entities"][entityList[i]]["spawnInfo"]["scaleDeviation"].asFloat();
				instancedEntity->spawnInfo.rotationDeviation.x = root["entities"][entityList[i]]["spawnInfo"]["rotationDeviation.x"].asFloat();
				instancedEntity->spawnInfo.rotationDeviation.y = root["entities"][entityList[i]]["spawnInfo"]["rotationDeviation.y"].asFloat();
				instancedEntity->spawnInfo.rotationDeviation.z = root["entities"][entityList[i]]["spawnInfo"]["rotationDeviation.z"].asFloat();

				if (root["entities"][entityList[i]]["snappedToTerrain"].asString() != "none")
				{
					FETerrain* terrain = SCENE.getTerrain(terrainNameToID[root["entities"][entityList[i]]["snappedToTerrain"].asString()]);
					terrain->snapInstancedEntity(instancedEntity);
				}

				instancedEntity->populate(instancedEntity->spawnInfo);

				if (root["entities"][entityList[i]]["modificationsToSpawn"].asBool())
				{
					std::ifstream infoFile;
					infoFile.open(projectFolder + instancedEntity->getObjectID() + ".txt");

					std::string infoFileData((std::istreambuf_iterator<char>(infoFile)), std::istreambuf_iterator<char>());

					Json::Value entityFileRoot;
					JSONCPP_STRING err;
					Json::CharReaderBuilder builder;

					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					if (!reader->parse(infoFileData.c_str(), infoFileData.c_str() + infoFileData.size(), &entityFileRoot, &err))
						return;

					size_t count = entityFileRoot["modifications"].size();
					for (int j = 0; j < count; j++)
					{
						if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_DELETED)
						{
							instancedEntity->deleteInstance(entityFileRoot["modifications"][j]["index"].asInt());
						}
						else if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_MODIFIED)
						{
							glm::mat4 modifedMatrix;
							for (int k = 0; k < 4; k++)
							{
								for (int p = 0; p < 4; p++)
								{
									modifedMatrix[k][p] = entityFileRoot["modifications"][j]["modification"][k][p].asFloat();
								}
							}

							instancedEntity->modifyInstance(entityFileRoot["modifications"][j]["index"].asInt(), modifedMatrix);
						}
						else if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_ADDED)
						{
							glm::mat4 modifedMatrix;
							for (int k = 0; k < 4; k++)
							{
								for (int p = 0; p < 4; p++)
								{
									modifedMatrix[k][p] = entityFileRoot["modifications"][j]["modification"][k][p].asFloat();
								}
							}

							instancedEntity->addInstance(modifedMatrix);
						}
					}
				}
			}
			else
			{
				SCENE.addEntity(RESOURCE_MANAGER.getGameModel(gameModelNameToID[root["entities"][entityList[i]]["gameModel"].asCString()]), entityList[i], root["entities"][entityList[i]]["ID"].asString());
				readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(root["entities"][entityList[i]]["ID"].asString())->transform);
			}
		}
		else
		{
			SCENE.addEntity(RESOURCE_MANAGER.getGameModel(gameModelNameToID[root["entities"][entityList[i]]["gameModel"].asCString()]), entityList[i], root["entities"][entityList[i]]["ID"].asString());
			readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(root["entities"][entityList[i]]["ID"].asString())->transform);
		}
	}

	// loading Lights
	std::vector<Json::String> lightList = root["lights"].getMemberNames();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		FELight* lightTest = SCENE.addLight(static_cast<FEObjectType>(root["lights"][lightList[i]]["type"].asInt() + 9), lightList[i]);
		FELight* light = SCENE.getLight(lightTest->getObjectID());

		// general light information
		light->setIntensity(root["lights"][lightList[i]]["intensity"].asFloat());
		readTransformToJSON(root["lights"][lightList[i]]["transformation"], &light->transform);
		light->setCastShadows(root["lights"][lightList[i]]["castShadows"].asBool());
		light->setLightEnabled(root["lights"][lightList[i]]["enabled"].asBool());
		light->setColor(glm::vec3(root["lights"][lightList[i]]["color"]["R"].asFloat(),
			root["lights"][lightList[i]]["color"]["G"].asFloat(),
			root["lights"][lightList[i]]["color"]["B"].asFloat()));
		light->setIsStaticShadowBias(root["lights"][lightList[i]]["staticShadowBias"].asBool());
		light->setShadowBias(root["lights"][lightList[i]]["shadowBias"].asFloat());
		light->setShadowBiasVariableIntensity(root["lights"][lightList[i]]["shadowBiasVariableIntensity"].asFloat());

		if (light->getType() == FE_POINT_LIGHT)
		{
			reinterpret_cast<FEPointLight*>(light)->setRange(root["lights"][lightList[i]]["range"].asFloat());
		}
		else if (light->getType() == FE_SPOT_LIGHT)
		{
			reinterpret_cast<FESpotLight*>(light)->setSpotAngle(root["lights"][lightList[i]]["spotAngle"].asFloat());
			reinterpret_cast<FESpotLight*>(light)->setSpotAngleOuter(root["lights"][lightList[i]]["spotAngleOuter"].asFloat());

			reinterpret_cast<FESpotLight*>(light)->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Z"].asFloat()));
		}
		else if (light->getType() == FE_DIRECTIONAL_LIGHT)
		{
			FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);

			directionalLight->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
				root["lights"][lightList[i]]["direction"]["Z"].asFloat()));

			directionalLight->setActiveCascades(root["lights"][lightList[i]]["CSM"]["activeCascades"].asInt());
			directionalLight->setShadowCoverage(root["lights"][lightList[i]]["CSM"]["shadowCoverage"].asFloat());
			directionalLight->setCSMZDepth(root["lights"][lightList[i]]["CSM"]["CSMZDepth"].asFloat());
			directionalLight->setCSMXYDepth(root["lights"][lightList[i]]["CSM"]["CSMXYDepth"].asFloat());
		}
	}

	// loading Effects settings
	// *********** Gamma Correction & Exposure ***********
	ENGINE.getCamera()->setGamma(root["effects"]["Gamma Correction & Exposure"]["Gamma"].asFloat());
	ENGINE.getCamera()->setExposure(root["effects"]["Gamma Correction & Exposure"]["Exposure"].asFloat());
	// *********** Anti-Aliasing(FXAA) ***********
	RENDERER.setFXAASpanMax(root["effects"]["Anti-Aliasing(FXAA)"]["FXAASpanMax"].asFloat());
	RENDERER.setFXAAReduceMin(root["effects"]["Anti-Aliasing(FXAA)"]["FXAAReduceMin"].asFloat());
	RENDERER.setFXAAReduceMul(root["effects"]["Anti-Aliasing(FXAA)"]["FXAAReduceMul"].asFloat());
	// *********** Bloom ***********
	//PPEffect = RENDERER.getPostProcessEffect("bloom");
	RENDERER.setBloomThreshold(root["effects"]["Bloom"]["thresholdBrightness"].asFloat());
	RENDERER.setBloomSize(root["effects"]["Bloom"]["BloomSize"].asFloat());
	// *********** Depth of Field ***********
	RENDERER.setDOFNearDistance(root["effects"]["Depth of Field"]["Near distance"].asFloat());
	RENDERER.setDOFFarDistance(root["effects"]["Depth of Field"]["Far distance"].asFloat());
	RENDERER.setDOFStrength(root["effects"]["Depth of Field"]["Strength"].asFloat());
	RENDERER.setDOFDistanceDependentStrength(root["effects"]["Depth of Field"]["Distance dependent strength"].asFloat());
	// *********** Distance fog ***********
	RENDERER.setDistanceFogEnabled(root["effects"]["Distance fog"]["Density"].asFloat() > -1.0f ? true : false);
	RENDERER.setDistanceFogDensity(root["effects"]["Distance fog"]["Density"].asFloat());
	RENDERER.setDistanceFogGradient(root["effects"]["Distance fog"]["Gradient"].asFloat());
	// *********** Chromatic Aberration ***********
	RENDERER.setChromaticAberrationIntensity(root["effects"]["Chromatic Aberration"]["Shift strength"].asFloat());
	// *********** Sky ***********
	RENDERER.setSkyEnabld(root["effects"]["Sky"]["Enabled"].asFloat() > 0.0f ? true : false);
	RENDERER.setDistanceToSky(root["effects"]["Sky"]["Sphere size"].asFloat());

	// loading Camera settings
	ENGINE.getCamera()->setPosition(glm::vec3(root["camera"]["position"]["X"].asFloat(),
		root["camera"]["position"]["Y"].asFloat(),
		root["camera"]["position"]["Z"].asFloat()));

	ENGINE.getCamera()->setFov(root["camera"]["fov"].asFloat());
	ENGINE.getCamera()->setNearPlane(root["camera"]["nearPlane"].asFloat());
	ENGINE.getCamera()->setFarPlane(root["camera"]["farPlane"].asFloat());

	ENGINE.getCamera()->setYaw(root["camera"]["yaw"].asFloat());
	ENGINE.getCamera()->setPitch(root["camera"]["pitch"].asFloat());
	ENGINE.getCamera()->setRoll(root["camera"]["roll"].asFloat());

	ENGINE.getCamera()->setAspectRatio(root["camera"]["aspectRatio"].asFloat());

	sceneFile.close();
}

bool FEProject::isModified()
{
	return modified;
}

void FEProject::setModified(bool newValue)
{
	modified = newValue;
}

void FEProject::addUnSavedObject(FEObject* object)
{
	unSavedObjects.push_back(object);
}

bool FEProject::shouldIncludeInSceneFile(FETexture* texture)
{
	// Terrain should manage it's textures in a different way.
	std::vector<std::string> terrainList = SCENE.getTerrainList();
	Json::Value terrainData;
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* terrain = SCENE.getTerrain(terrainList[i]);
		if (terrain->heightMap->getObjectID() == texture->getObjectID())
			return false;

		if (terrain->layerMaps[0] != nullptr && terrain->layerMaps[0]->getObjectID() == texture->getObjectID())
			return false;

		if (terrain->layerMaps[1] != nullptr && terrain->layerMaps[1]->getObjectID() == texture->getObjectID())
			return false;
	}

	return true;
}

void FEProject::setProjectFolder(std::string newValue)
{
	if (!FILE_SYSTEM.isFolder(newValue.c_str()))
		return;

	projectFolder = newValue;
}

void FEProject::saveSceneTo(std::string newPath)
{
	if (!FILE_SYSTEM.isFolder(newPath.c_str()))
		return;

	setProjectFolder(newPath);
	ENGINE.takeScreenshot((getProjectFolder() + "projectScreenShot.texture").c_str());
	saveScene(true);
}