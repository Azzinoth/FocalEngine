#include "FEProject.h"

FEProject::FEProject(std::string Name, std::string ProjectFolder)
{
	name = Name;
	projectFolder = ProjectFolder;

	std::ifstream screenshotFile((this->getProjectFolder() + "/projectScreenShot.FETexture").c_str());
	
	if (!screenshotFile.good())
	{
		sceneScreenshot = RESOURCE_MANAGER.noTexture;
	}
	else
	{
		sceneScreenshot = RESOURCE_MANAGER.LoadFETextureStandAlone((this->getProjectFolder() + "/projectScreenShot.FETexture").c_str());
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

		if (mesh->getFileName().size() == 0)
			continue;

		meshData[mesh->getName()] = mesh->getName() + ".model";
	}
	root["meshes"] = meshData;

	// saving Textures
	std::vector<std::string> texturesList = RESOURCE_MANAGER.getTextureList();
	Json::Value texturesData;
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		FETexture* texture = RESOURCE_MANAGER.getTexture(texturesList[i]);
		texturesData[texture->getName()] = texture->getName() + ".FETexture";
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
		
		terrainData[terrain->getName()]["heightMap"] = terrain->heightMap->getName();
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

			//lightData[directionalLight->getName()]["type"] = directionalLight->getType();
			//lightData[directionalLight->getName()]["intensity"] = directionalLight->getIntensity();
			//lightData[directionalLight->getName()]["range"] = 0.0f;
			//lightData[directionalLight->getName()]["spotAngle"] = 0.0f;
			//lightData[directionalLight->getName()]["spotAngleOuter"] = 0.0f;
			//lightData[directionalLight->getName()]["castShadows"] = directionalLight->isCastShadows();
			//lightData[directionalLight->getName()]["enabled"] = directionalLight->isLightEnabled();
			//lightData[directionalLight->getName()]["color"]["R"] = directionalLight->getColor()[0];
			//lightData[directionalLight->getName()]["color"]["G"] = directionalLight->getColor()[1];
			//lightData[directionalLight->getName()]["color"]["B"] = directionalLight->getColor()[2];
			//lightData[directionalLight->getName()]["staticShadowBias"] = directionalLight->isStaticShadowBias();
			//writeTransformToJSON(lightData[light->getName()]["transformation"], &directionalLight->transform);
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

			//lightData[spotLight->getName()]["type"] = spotLight->getType();
			//lightData[spotLight->getName()]["intensity"] = spotLight->getIntensity();
			//lightData[spotLight->getName()]["range"] = spotLight->getRange();
			lightData[spotLight->getName()]["spotAngle"] = spotLight->getSpotAngle();
			lightData[spotLight->getName()]["spotAngleOuter"] = spotLight->getSpotAngleOuter();
			//lightData[spotLight->getName()]["castShadows"] = spotLight->isCastShadows();
			//lightData[spotLight->getName()]["enabled"] = spotLight->isLightEnabled();
			//lightData[spotLight->getName()]["color"]["R"] = spotLight->getColor()[0];
			//lightData[spotLight->getName()]["color"]["G"] = spotLight->getColor()[1];
			//lightData[spotLight->getName()]["color"]["B"] = spotLight->getColor()[2];
			//writeTransformToJSON(lightData[light->getName()]["transformation"], &spotLight->transform);
			lightData[spotLight->getName()]["direction"]["X"] = spotLight->getDirection()[0];
			lightData[spotLight->getName()]["direction"]["Y"] = spotLight->getDirection()[1];
			lightData[spotLight->getName()]["direction"]["Z"] = spotLight->getDirection()[2];
		}
		else if (light->getType() == FE_POINT_LIGHT)
		{
			FEPointLight* pointLight = reinterpret_cast<FEPointLight*>(light);

			//lightData[pointLight->getName()]["type"] = pointLight->getType();
			//lightData[pointLight->getName()]["intensity"] = pointLight->getIntensity();
			lightData[pointLight->getName()]["range"] = pointLight->getRange();
			//lightData[pointLight->getName()]["spotAngle"] = 0.0f;
			//lightData[pointLight->getName()]["spotAngleOuter"] = 0.0f;
			//lightData[pointLight->getName()]["castShadows"] = pointLight->isCastShadows();
			//lightData[pointLight->getName()]["enabled"] = pointLight->isLightEnabled();
			//lightData[pointLight->getName()]["color"]["R"] = pointLight->getColor()[0];
			//lightData[pointLight->getName()]["color"]["G"] = pointLight->getColor()[1];
			//lightData[pointLight->getName()]["color"]["B"] = pointLight->getColor()[2];
			//writeTransformToJSON(lightData[light->getName()]["transformation"], &pointLight->transform);
			//lightData[pointLight->getName()]["direction"]["X"] = 0.0f;
			//lightData[pointLight->getName()]["direction"]["Y"] = 0.0f;
			//lightData[pointLight->getName()]["direction"]["Z"] = 0.0f;
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
		RESOURCE_MANAGER.LoadFEMesh((projectFolder + root["meshes"][meshList[i]].asCString()).c_str(), meshList[i]);
	}

	// loading Textures
	std::vector<Json::String> texturesList = root["textures"].getMemberNames();
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		FETexture* justLoadedTexture = RESOURCE_MANAGER.LoadFETexture((projectFolder + root["textures"][texturesList[i]].asCString()).c_str(), texturesList[i]);
		// I have texture name in file but for now I will only use name from scene.txt
		RESOURCE_MANAGER.setTextureName(justLoadedTexture, texturesList[i]);
	}

	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = RESOURCE_MANAGER.createMaterial(materialsList[i].c_str());
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
										 gameModelList[i]);
	}

	// loading Entities
	std::vector<Json::String> entityList = root["entities"].getMemberNames();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		SCENE.addEntity(RESOURCE_MANAGER.getGameModel(root["entities"][entityList[i]]["gameModel"].asCString()), entityList[i]);
		readTransformToJSON(root["entities"][entityList[i]]["transformation"], &SCENE.getEntity(entityList[i])->transform);
	}

	// loading Terrains
	std::vector<Json::String> terrainList = root["terrains"].getMemberNames();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* terrainSample = RESOURCE_MANAGER.createTerrain("test");
		terrainSample->heightMap = RESOURCE_MANAGER.getTexture(root["terrains"][terrainList[i]]["heightMap"].asCString());
		terrainSample->layer0 = RESOURCE_MANAGER.getMaterial(root["terrains"][terrainList[i]]["materials"]["layer0"].asCString());

		terrainSample->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		terrainSample->setDisplacementScale(root["terrains"][terrainList[i]]["displacementScale"].asFloat());
		glm::vec2 tileMult;
		tileMult.x = root["terrains"][terrainList[i]]["tileMult"]["X"].asFloat();
		tileMult.y = root["terrains"][terrainList[i]]["tileMult"]["Y"].asFloat();
		terrainSample->setTileMult(tileMult);
		terrainSample->setLODlevel(root["terrains"][terrainList[i]]["LODlevel"].asFloat());
		terrainSample->setChunkPerSide(root["terrains"][terrainList[i]]["chunkPerSide"].asFloat());
		terrainSample->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		terrainSample->setHightScale(root["terrains"][terrainList[i]]["hightScale"].asFloat());
		readTransformToJSON(root["terrains"][terrainList[i]]["transformation"], &terrainSample->transform);

		SCENE.addTerrain(terrainSample);
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

	RESOURCE_MANAGER.saveFETexture((getProjectFolder() + "/projectScreenShot.FETexture").c_str(), pixels, width, height);
	delete[] pixels;
}