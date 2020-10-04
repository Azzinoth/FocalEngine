#include "FEProject.h"

bool deleteFile(const char* filePath);

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

void FEProject::saveScene(std::unordered_map<int, FEEntity*>* excludedEntities)
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
	}
	root["textures"] = texturesData;

	// saving Materials
	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();
	Json::Value materialData;
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FEMaterial* material = RESOURCE_MANAGER.getMaterial(materialList[i]);

		if (material->albedoMap != nullptr) materialData[material->getName()]["albedoMap"] = material->albedoMap->getName();
		if (material->normalMap != nullptr) materialData[material->getName()]["normalMap"] = material->normalMap->getName();
		if (material->roughtnessMap != nullptr) materialData[material->getName()]["roughtnessMap"] = material->roughtnessMap->getName();
		if (material->metalnessMap != nullptr) materialData[material->getName()]["metalnessMap"] = material->metalnessMap->getName();
		if (material->AOMap != nullptr) materialData[material->getName()]["AOMap"] = material->AOMap->getName();
		if (material->displacementMap != nullptr) materialData[material->getName()]["displacementMap"] = material->displacementMap->getName();

		materialData[material->getName()]["ID"] = material->getAssetID();
		materialData[material->getName()]["name"] = material->getName();
		materialData[material->getName()]["metalness"] = material->getMetalness();
		materialData[material->getName()]["roughtness"] = material->getRoughtness();
		materialData[material->getName()]["normalMapIntensity"] = material->getNormalMapIntensity();
		materialData[material->getName()]["ambientOcclusionIntensity"] = material->getAmbientOcclusionIntensity();
		materialData[material->getName()]["ambientOcclusionMapIntensity"] = material->getAmbientOcclusionMapIntensity();
		materialData[material->getName()]["roughtnessMapIntensity"] = material->getRoughtnessMapIntensity();
		materialData[material->getName()]["metalnessMapIntensity"] = material->getMetalnessMapIntensity();
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
	}
	root["gameModels"] = gameModelData;

	// saving Entities
	std::vector<std::string> entityList = SCENE.getEntityList();
	Json::Value entityData;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = SCENE.getEntity(entityList[i]);
		if (excludedEntities->find(entity->getNameHash()) != excludedEntities->end())
			continue;

		entityData[entity->getName()]["ID"] = entity->getAssetID();
		entityData[entity->getName()]["name"] = entity->getName();
		entityData[entity->getName()]["gameModel"] = entity->gameModel->getName();
		writeTransformToJSON(entityData[entity->getName()]["transformation"], &entity->transform);
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

	cameraData["gamma"] = ENGINE.getCamera()->getGamma();
	cameraData["exposure"] = ENGINE.getCamera()->getExposure();

	root["camera"] = cameraData;

	// saving into file
	Json::StreamWriterBuilder builder;
	const std::string json_file = Json::writeString(builder, root);

	sceneFile << json_file;
	sceneFile.close();
	
	for (size_t i = 0; i < filesToDelete.size(); i++)
	{
		deleteFile(filesToDelete[i].c_str());
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

		FETexture* loadedTexture = RESOURCE_MANAGER.LoadFETexture((projectFolder + root["textures"][texturesList[i]]["fileName"].asCString()).c_str(), root["textures"][texturesList[i]]["name"].asString());
	}

	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = RESOURCE_MANAGER.createMaterial(materialsList[i].c_str(), root["materials"][materialsList[i]]["ID"].asString());
		//newMat->shader = RESOURCE_MANAGER.getShader("FEPhongShader");
		if (newMat->getName() == "skyDome")
		{
			newMat->shader = RESOURCE_MANAGER.getShader("FESkyDome");
		}
		else
		{
			newMat->shader = RESOURCE_MANAGER.getShader("FEPBRShader");
		}

		std::vector<Json::String> textureList = root["materials"][materialsList[i]].getMemberNames();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			if (textureList[j] == "albedoMap")
				newMat->albedoMap = RESOURCE_MANAGER.getTexture(root["materials"][materialsList[i]]["albedoMap"].asCString());

			if (textureList[j] == "normalMap")
				newMat->normalMap = RESOURCE_MANAGER.getTexture(root["materials"][materialsList[i]]["normalMap"].asCString());

			if (textureList[j] == "AOMap")
				newMat->AOMap = RESOURCE_MANAGER.getTexture(root["materials"][materialsList[i]]["AOMap"].asCString());

			if (textureList[j] == "roughtnessMap")
				newMat->roughtnessMap = RESOURCE_MANAGER.getTexture(root["materials"][materialsList[i]]["roughtnessMap"].asCString());

			if (textureList[j] == "metalnessMap")
				newMat->metalnessMap = RESOURCE_MANAGER.getTexture(root["materials"][materialsList[i]]["metalnessMap"].asCString());

			if (textureList[j] == "displacementMap")
				newMat->displacementMap = RESOURCE_MANAGER.getTexture(root["materials"][materialsList[i]]["displacementMap"].asCString());

			newMat->setMetalness(root["materials"][materialsList[i]]["metalness"].asFloat());
			newMat->setRoughtness(root["materials"][materialsList[i]]["roughtness"].asFloat());
			newMat->setNormalMapIntensity(root["materials"][materialsList[i]]["normalMapIntensity"].asFloat());
			newMat->setAmbientOcclusionIntensity(root["materials"][materialsList[i]]["ambientOcclusionIntensity"].asFloat());
			newMat->setAmbientOcclusionMapIntensity(root["materials"][materialsList[i]]["ambientOcclusionMapIntensity"].asFloat());
			newMat->setRoughtnessMapIntensity(root["materials"][materialsList[i]]["roughtnessMapIntensity"].asFloat());
			newMat->setMetalnessMapIntensity(root["materials"][materialsList[i]]["metalnessMapIntensity"].asFloat());
		}
	}

	// loading gameModels
	std::vector<Json::String> gameModelList = root["gameModels"].getMemberNames();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		RESOURCE_MANAGER.createGameModel(RESOURCE_MANAGER.getMesh(root["gameModels"][gameModelList[i]]["mesh"].asCString()),
										 RESOURCE_MANAGER.getMaterial(root["gameModels"][gameModelList[i]]["material"].asCString()),
										 gameModelList[i], root["gameModels"][gameModelList[i]]["ID"].asString());
	}

	// loading Entities
	std::vector<Json::String> entityList = root["entities"].getMemberNames();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		SCENE.addEntity(RESOURCE_MANAGER.getGameModel(root["entities"][entityList[i]]["gameModel"].asCString()), entityList[i], root["entities"][entityList[i]]["ID"].asString());
		readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);
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

	ENGINE.getCamera()->setGamma(root["camera"]["gamma"].asFloat());
	ENGINE.getCamera()->setExposure(root["camera"]["exposure"].asFloat());

	sceneFile.close();
}

void FEProject::createDummyScreenshot()
{
	size_t width = ENGINE.getWindowWidth();
	size_t height = ENGINE.getWindowHeight();

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