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
	if (!FILESYSTEM.checkFolder(PROJECTS_FOLDER))
		FILESYSTEM.createFolder(PROJECTS_FOLDER);

	std::vector<std::string> projectNameList = FILESYSTEM.getFolderList(PROJECTS_FOLDER);

	for (size_t i = 0; i < projectNameList.size(); i++)
	{
		list.push_back(new FEProject(projectNameList[i].c_str(), std::string(PROJECTS_FOLDER) + std::string("/") + projectNameList[i].c_str() + "/"));
	}
}

void FEProjectManager::closeCurrentProject()
{
	//closing all windows or popups.
	WindowsManager::getInstance().closeAllWindows();
	WindowsManager::getInstance().closeAllPopups();

	for (size_t i = 0; i < list.size(); i++)
	{
		delete list[i];
	}
	list.clear();
	PROJECT_MANAGER.setCurrent(nullptr);
	PREVIEW_MANAGER.clear();

	loadProjectList();
	SELECTED.clear();
}

void FEProjectManager::openProject(int projectIndex)
{
	PROJECT_MANAGER.setCurrent(list[projectIndex]);
	PROJECT_MANAGER.getCurrent()->loadScene();
	indexChosen = -1;

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
		entity->transform.setDirty(false);
	}
}

void FEProjectManager::displayProjectSelection()
{
	float mainWindowW = float(ENGINE.getWindowWidth());
	float mainWindowH = float(ENGINE.getWindowHeight());

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(mainWindowW, mainWindowH - 170.0f));
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
			ImGui::PushID(i);
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
				indexChosen = i;
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
	ImGui::SetNextWindowPos(ImVec2(0.0f, mainWindowH - 170.0f));
	ImGui::SetNextWindowSize(ImVec2(mainWindowW, 170.0f));
	ImGui::Begin("##create project", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.75f, 0.70f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		if (ImGui::Button("Create New Project", ImVec2(200.0f, 64.0f)))
			ImGui::OpenPopup("New Project");

		ImGui::SameLine();
		if (ImGui::Button("Open Project", ImVec2(200.0f, 64.0f)) && indexChosen != -1)
		{
			openProject(indexChosen);
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Project", ImVec2(200.0f, 64.0f)) && indexChosen != -1)
		{
			std::string projectFolder = list[indexChosen]->getProjectFolder();
			projectFolder.erase(projectFolder.begin() + projectFolder.size() - 1);

			// geting list of all files and folders in project folder
			auto fileList = FILESYSTEM.getFolderList(list[indexChosen]->getProjectFolder().c_str());
			// we would delete all files in project folder, my editor would not create folders there
			// so we are deleting only files.
			for (size_t i = 0; i < fileList.size(); i++)
			{
				FILESYSTEM.deleteFile((list[indexChosen]->getProjectFolder() + fileList[i]).c_str());
			}
			// then we can try to delete project folder, but if user created some folders in it we will fail.
			FILESYSTEM.deleteFolder(projectFolder.c_str());

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

		if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Insert name of new project :");
			static char projectName[512] = "";

			ImGui::InputText("", projectName, IM_ARRAYSIZE(projectName));
			ImGui::Separator();

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
					FILESYSTEM.createFolder((std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/").c_str());
					list.push_back(new FEProject(projectName, std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/"));
					list.back()->createDummyScreenshot();
					SCENE.addLight(FE_DIRECTIONAL_LIGHT, "sun");
					ImGui::CloseCurrentPopup();
					strcpy_s(projectName, "");
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
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

void FEProject::saveScene()
{
	Json::Value root;
	std::ofstream sceneFile;
	sceneFile.open(projectFolder + "scene.txt");

	// saving Meshes
	std::vector<std::string> meshList = RESOURCE_MANAGER.getMeshList();
	Json::Value meshData;
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FEMesh* mesh = RESOURCE_MANAGER.getMesh(meshList[i]);

		if (mesh->getDirtyFlag())
			RESOURCE_MANAGER.saveFEMesh(mesh, (getProjectFolder() + mesh->getAssetID() + std::string(".model")).c_str());
		meshData[mesh->getName()]["ID"] = mesh->getAssetID();
		meshData[mesh->getName()]["name"] = mesh->getName();
		meshData[mesh->getName()]["fileName"] = mesh->getAssetID() + ".model";

		mesh->setDirtyFlag(false);
	}
	root["meshes"] = meshData;

	// saving Textures
	std::vector<std::string> texturesList = RESOURCE_MANAGER.getTextureList();
	Json::Value texturesData;
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		FETexture* texture = RESOURCE_MANAGER.getTexture(texturesList[i]);

		if (texture->getDirtyFlag())
			RESOURCE_MANAGER.saveFETexture(texture, (getProjectFolder() + texture->getAssetID() + std::string(".texture")).c_str());
		texturesData[texture->getName()]["ID"] = texture->getAssetID();
		texturesData[texture->getName()]["name"] = texture->getName();
		texturesData[texture->getName()]["fileName"] = texture->getAssetID() + ".texture";
		texturesData[texture->getName()]["type"] = texture->getInternalFormat();

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
				materialData[material->getName()]["textures"][std::to_string(j).c_str()] = material->textures[j]->getName();

			if (material->textureBindings[j] != -1)
				materialData[material->getName()]["textureBindings"][std::to_string(j).c_str()] = material->textureBindings[j];

			if (material->textureChannels[j] != -1)
				materialData[material->getName()]["textureChannels"][std::to_string(j).c_str()] = material->textureChannels[j];
		}

		materialData[material->getName()]["ID"] = material->getAssetID();
		materialData[material->getName()]["name"] = material->getName();
		materialData[material->getName()]["metalness"] = material->getMetalness();
		materialData[material->getName()]["roughtness"] = material->getRoughtness();
		materialData[material->getName()]["normalMapIntensity"] = material->getNormalMapIntensity();
		materialData[material->getName()]["ambientOcclusionIntensity"] = material->getAmbientOcclusionIntensity();
		materialData[material->getName()]["ambientOcclusionMapIntensity"] = material->getAmbientOcclusionMapIntensity();
		materialData[material->getName()]["roughtnessMapIntensity"] = material->getRoughtnessMapIntensity();
		materialData[material->getName()]["metalnessMapIntensity"] = material->getMetalnessMapIntensity();

		material->setDirtyFlag(false);
	}
	root["materials"] = materialData;

	// saving gameModels
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	Json::Value gameModelData;
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* gameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);

		gameModelData[gameModel->getName()]["ID"] = gameModel->getAssetID();
		gameModelData[gameModel->getName()]["name"] = gameModel->getName();
		gameModelData[gameModel->getName()]["mesh"] = gameModel->mesh->getName();
		gameModelData[gameModel->getName()]["material"] = gameModel->material->getName();
		gameModelData[gameModel->getName()]["scaleFactor"] = gameModel->getScaleFactor();

		gameModelData[gameModel->getName()]["LODs"]["haveLODlevels"] = gameModel->useLODlevels();
		if (gameModel->useLODlevels())
		{
			gameModelData[gameModel->getName()]["LODs"]["cullDistance"] = gameModel->getCullDistance();
			gameModelData[gameModel->getName()]["LODs"]["billboardZeroRotaion"] = gameModel->getBillboardZeroRotaion();
			gameModelData[gameModel->getName()]["LODs"]["LODCount"] = gameModel->getLODCount();
			for (size_t j = 0; j < gameModel->getLODCount(); j++)
			{
				gameModelData[gameModel->getName()]["LODs"][std::to_string(j)]["mesh"] = gameModel->getLODMesh(j)->getName();
				gameModelData[gameModel->getName()]["LODs"][std::to_string(j)]["maxDrawDistance"] = gameModel->getLODMaxDrawDistance(j);
				gameModelData[gameModel->getName()]["LODs"][std::to_string(j)]["isBillboard"] = gameModel->isLODBillboard(j);
				if (gameModel->isLODBillboard(j))
					gameModelData[gameModel->getName()]["LODs"][std::to_string(j)]["billboardMaterial"] = gameModel->getBillboardMaterial()->getName();
			}
		}
		
		gameModel->setDirtyFlag(false);
	}
	root["gameModels"] = gameModelData;

	// saving Entities
	std::vector<std::string> entityList = SCENE.getEntityList();
	Json::Value entityData;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = SCENE.getEntity(entityList[i]);
		if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(entity))
			continue;

		entityData[entity->getName()]["ID"] = entity->getAssetID();
		entityData[entity->getName()]["type"] = FEAssetTypeToString(entity->getType());
		entityData[entity->getName()]["name"] = entity->getName();
		entityData[entity->getName()]["gameModel"] = entity->gameModel->getName();
		writeTransformToJSON(entityData[entity->getName()]["transformation"], &entity->transform);

		if (entity->getType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* instancedEntity = reinterpret_cast<FEEntityInstanced*>(entity);
			entityData[entity->getName()]["modificationsToSpawn"] = instancedEntity->getSpawnModificationCount() == 0 ? false : true;
			if (instancedEntity->getSpawnModificationCount())
			{
				std::ofstream infoFile;
				Json::Value entityFileRoot;
				infoFile.open(projectFolder + entity->getAssetID() + ".txt");

				Json::Value modificationsData;
				auto modificationList = instancedEntity->getSpawnModifications();
				for (size_t j = 0; j < modificationList.size(); j++)
				{
					modificationsData[j]["type"] = int(modificationList[j].type);
					modificationsData[j]["index"] = modificationList[j].index;
					if (modificationList[j].type != CHANGE_DELETED)
					{
						for (size_t k = 0; k < 4; k++)
						{
							for (size_t p = 0; p < 4; p++)
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

			entityData[entity->getName()]["spawnInfo"]["seed"] = instancedEntity->spawnInfo.seed;
			entityData[entity->getName()]["spawnInfo"]["objectCount"] = instancedEntity->spawnInfo.objectCount;
			entityData[entity->getName()]["spawnInfo"]["radius"] = instancedEntity->spawnInfo.radius;
			entityData[entity->getName()]["spawnInfo"]["scaleDeviation"] = instancedEntity->spawnInfo.scaleDeviation;
			entityData[entity->getName()]["spawnInfo"]["rotationDeviation.x"] = instancedEntity->spawnInfo.rotationDeviation.x;
			entityData[entity->getName()]["spawnInfo"]["rotationDeviation.y"] = instancedEntity->spawnInfo.rotationDeviation.y;
			entityData[entity->getName()]["spawnInfo"]["rotationDeviation.z"] = instancedEntity->spawnInfo.rotationDeviation.z;
			if (instancedEntity->getSnappedToTerrain() == nullptr)
			{
				entityData[entity->getName()]["snappedToTerrain"] = "none";
			}
			else
			{
				entityData[entity->getName()]["snappedToTerrain"] = instancedEntity->getSnappedToTerrain()->getName();
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

		terrainData[terrain->getName()]["ID"] = terrain->getAssetID();
		terrainData[terrain->getName()]["name"] = terrain->getName();
		terrainData[terrain->getName()]["heightMap"]["name"] = terrain->heightMap->getName();
		terrainData[terrain->getName()]["heightMap"]["fileName"] = terrain->heightMap->getAssetID() + ".texture";
		terrainData[terrain->getName()]["hightScale"] = terrain->getHightScale();
		terrainData[terrain->getName()]["displacementScale"] = terrain->getDisplacementScale();
		terrainData[terrain->getName()]["tileMult"]["X"] = terrain->getTileMult().x;
		terrainData[terrain->getName()]["tileMult"]["Y"] = terrain->getTileMult().y;
		terrainData[terrain->getName()]["LODlevel"] = terrain->getLODlevel();
		terrainData[terrain->getName()]["chunkPerSide"] = terrain->getChunkPerSide();
		terrainData[terrain->getName()]["materials"]["layer0"] = terrain->layer0->getName();

		writeTransformToJSON(terrainData[terrain->getName()]["transformation"], &terrain->transform);

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
		lightData[light->getName()]["type"] = light->getType();
		lightData[light->getName()]["intensity"] = light->getIntensity();
		writeTransformToJSON(lightData[light->getName()]["transformation"], &light->transform);
		lightData[light->getName()]["castShadows"] = light->isCastShadows();
		lightData[light->getName()]["enabled"] = light->isLightEnabled();
		lightData[light->getName()]["color"]["R"] = light->getColor()[0];
		lightData[light->getName()]["color"]["G"] = light->getColor()[1];
		lightData[light->getName()]["color"]["B"] = light->getColor()[2];
		lightData[light->getName()]["staticShadowBias"] = light->isStaticShadowBias();
		lightData[light->getName()]["shadowBias"] = light->getShadowBias(); 
		lightData[light->getName()]["shadowBiasVariableIntensity"] = light->getShadowBiasVariableIntensity();

		// type specific information
		if (light->getType() == FE_DIRECTIONAL_LIGHT)
		{
			FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);

			lightData[directionalLight->getName()]["direction"]["X"] = directionalLight->getDirection()[0];
			lightData[directionalLight->getName()]["direction"]["Y"] = directionalLight->getDirection()[1];
			lightData[directionalLight->getName()]["direction"]["Z"] = directionalLight->getDirection()[2];
			lightData[directionalLight->getName()]["CSM"]["activeCascades"] = directionalLight->getActiveCascades();
			lightData[directionalLight->getName()]["CSM"]["shadowCoverage"] = directionalLight->getShadowCoverage();
			lightData[directionalLight->getName()]["CSM"]["CSMZDepth"] = directionalLight->getCSMZDepth();
			lightData[directionalLight->getName()]["CSM"]["CSMXYDepth"] = directionalLight->getCSMXYDepth();
		}
		else if (light->getType() == FE_SPOT_LIGHT)
		{
			FESpotLight* spotLight = reinterpret_cast<FESpotLight*>(light);

			lightData[spotLight->getName()]["spotAngle"] = spotLight->getSpotAngle();
			lightData[spotLight->getName()]["spotAngleOuter"] = spotLight->getSpotAngleOuter();
			lightData[spotLight->getName()]["direction"]["X"] = spotLight->getDirection()[0];
			lightData[spotLight->getName()]["direction"]["Y"] = spotLight->getDirection()[1];
			lightData[spotLight->getName()]["direction"]["Z"] = spotLight->getDirection()[2];
		}
		else if (light->getType() == FE_POINT_LIGHT)
		{
			FEPointLight* pointLight = reinterpret_cast<FEPointLight*>(light);

			lightData[pointLight->getName()]["range"] = pointLight->getRange();
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

	root["camera"] = cameraData;

	// saving into file
	Json::StreamWriterBuilder builder;
	const std::string json_file = Json::writeString(builder, root);

	sceneFile << json_file;
	sceneFile.close();
	
	for (size_t i = 0; i < filesToDelete.size(); i++)
	{
		FILESYSTEM.deleteFile(filesToDelete[i].c_str());
	}

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

	Json::Value root;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &root, &err))
		return;

	// loading Meshes
	std::vector<Json::String> meshList = root["meshes"].getMemberNames();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		RESOURCE_MANAGER.LoadFEMesh((projectFolder + root["meshes"][meshList[i]]["fileName"].asCString()).c_str(), root["meshes"][meshList[i]]["name"].asCString());
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

		FETexture* loadedTexture = RESOURCE_MANAGER.LoadFETextureAsync((projectFolder + root["textures"][texturesList[i]]["fileName"].asCString()).c_str(), root["textures"][texturesList[i]]["name"].asString());
	}

	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = RESOURCE_MANAGER.createMaterial(materialsList[i], root["materials"][materialsList[i]]["ID"].asString());

		//newMat->shader = RESOURCE_MANAGER.getShader("FEPhongShader");
		//newMat->shader = RESOURCE_MANAGER.getShader("FESolidColorShader");
		newMat->shader = RESOURCE_MANAGER.getShader("FEPBRShader");

		std::vector<Json::String> membersList = root["materials"][materialsList[i]].getMemberNames();
		for (size_t j = 0; j < membersList.size(); j++)
		{
			if (membersList[j] == "textures")
			{
				for (size_t k = 0; k < FE_MAX_TEXTURES_PER_MATERIAL; k++)
				{
					if (root["materials"][materialsList[i]]["textures"].isMember(std::to_string(k).c_str()))
					{
						std::string textureName = root["materials"][materialsList[i]]["textures"][std::to_string(k).c_str()].asCString();
						newMat->textures[k] = RESOURCE_MANAGER.getTexture(textureName);
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

	// loading gameModels
	std::vector<Json::String> gameModelList = root["gameModels"].getMemberNames();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* newGameModel = RESOURCE_MANAGER.createGameModel(RESOURCE_MANAGER.getMesh(root["gameModels"][gameModelList[i]]["mesh"].asCString()),
																	 RESOURCE_MANAGER.getMaterial(root["gameModels"][gameModelList[i]]["material"].asCString()),
																	 gameModelList[i], root["gameModels"][gameModelList[i]]["ID"].asString());
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

	// loading Terrains
	std::vector<Json::String> terrainList = root["terrains"].getMemberNames();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* newTerrain = RESOURCE_MANAGER.createTerrain(false, terrainList[i], root["terrains"][terrainList[i]]["ID"].asString());
		newTerrain->heightMap = RESOURCE_MANAGER.LoadFEHeightmap((projectFolder + root["terrains"][terrainList[i]]["heightMap"]["fileName"].asCString()).c_str(), newTerrain, root["terrains"][terrainList[i]]["heightMap"]["name"].asCString());
		newTerrain->layer0 = RESOURCE_MANAGER.getMaterial(root["terrains"][terrainList[i]]["materials"]["layer0"].asCString());

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
	for (size_t i = 0; i < entityList.size(); i++)
	{
		if (root["entities"][entityList[i]].isMember("type"))
		{
			if (root["entities"][entityList[i]]["type"] == "FE_ENTITY_INSTANCED")
			{
				FEEntityInstanced* instancedEntity = SCENE.addEntityInstanced(RESOURCE_MANAGER.getGameModel(root["entities"][entityList[i]]["gameModel"].asCString()), entityList[i], root["entities"][entityList[i]]["ID"].asString());
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
					infoFile.open(projectFolder + instancedEntity->getAssetID() + ".txt");

					std::string infoFileData((std::istreambuf_iterator<char>(infoFile)), std::istreambuf_iterator<char>());

					Json::Value entityFileRoot;
					JSONCPP_STRING err;
					Json::CharReaderBuilder builder;

					const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
					if (!reader->parse(infoFileData.c_str(), infoFileData.c_str() + infoFileData.size(), &entityFileRoot, &err))
						return;
					
					size_t count = entityFileRoot["modifications"].size();
					for (size_t j = 0; j < count; j++)
					{
						if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_DELETED)
						{
							instancedEntity->deleteInstance(entityFileRoot["modifications"][j]["index"].asInt());
						}
						else if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_MODIFIED)
						{
							glm::mat4 modifedMatrix;
							for (size_t k = 0; k < 4; k++)
							{
								for (size_t p = 0; p < 4; p++)
								{
									modifedMatrix[k][p] = entityFileRoot["modifications"][j]["modification"][k][p].asFloat();
								}
							}

							instancedEntity->modifyInstance(entityFileRoot["modifications"][j]["index"].asInt(), modifedMatrix);
						}
						else if (entityFileRoot["modifications"][j]["type"].asInt() == CHANGE_ADDED)
						{
							glm::mat4 modifedMatrix;
							for (size_t k = 0; k < 4; k++)
							{
								for (size_t p = 0; p < 4; p++)
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
				SCENE.addEntity(RESOURCE_MANAGER.getGameModel(root["entities"][entityList[i]]["gameModel"].asCString()), entityList[i], root["entities"][entityList[i]]["ID"].asString());
				readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);
			}
		}
		else
		{
			SCENE.addEntity(RESOURCE_MANAGER.getGameModel(root["entities"][entityList[i]]["gameModel"].asCString()), entityList[i], root["entities"][entityList[i]]["ID"].asString());
			readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);
		}
	}

	// loading Lights
	std::vector<Json::String> lightList = root["lights"].getMemberNames();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		SCENE.addLight(static_cast<FELightType>(root["lights"][lightList[i]]["type"].asInt()), lightList[i]);
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
	ENGINE.getCamera()->setGamma(root["effects"]["Gamma Correction & Exposure"] ["Gamma"].asFloat());
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

void FEProject::createDummyScreenshot()
{
	size_t width = ENGINE.getRenderTargetWidth();
	size_t height = ENGINE.getRenderTargetHeight();

	char* pixels = new char[4 * width * height];
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

	FETexture* tempTexture = RESOURCE_MANAGER.rawDataToFETexture(pixels, width, height);
	RESOURCE_MANAGER.saveFETexture(tempTexture, (getProjectFolder() + "/projectScreenShot.texture").c_str());
	RESOURCE_MANAGER.deleteFETexture(tempTexture);
	delete[] pixels;
}

void FEProject::addFileToDeleteList(std::string fileName)
{
	filesToDelete.push_back(fileName);
}