#include "FEProject.h"

FEProject::FEProject(std::string Name, std::string ProjectFolder)
{
	name = Name;
	projectFolder = ProjectFolder;

	std::ifstream screenshotFile((this->getProjectFolder() + "/projectScreenShot.FETexture").c_str());
	/*if (!screenshotFile.good())
		createDummyScreenshot();
	
	sceneScreenshot = FEResourceManager::getInstance().LoadFETexture((this->getProjectFolder() + "/projectScreenShot.texture").c_str());*/
	if (!screenshotFile.good())
	{
		sceneScreenshot = FEResourceManager::getInstance().noTexture;
	}
	else
	{
		sceneScreenshot = FEResourceManager::getInstance().LoadFETextureStandAlone((this->getProjectFolder() + "/projectScreenShot.FETexture").c_str());
	}

	screenshotFile.close();
}

FEProject::~FEProject()
{
	if (sceneScreenshot != FEResourceManager::getInstance().noTexture)
		delete sceneScreenshot;
	FEScene::getInstance().clear();
	FEResourceManager::getInstance().clear();
	FEngine::getInstance().resetCamera();
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
	FEngine& engine = FEngine::getInstance();
	FEResourceManager& resourceManager = FEResourceManager::getInstance();
	FEScene& scene = FEScene::getInstance();

	Json::Value root;
	std::ofstream sceneFile;
	sceneFile.open(projectFolder + "scene.txt");

	// saving Meshes
	std::vector<std::string> meshList = resourceManager.getMeshList();
	Json::Value meshData;
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FEMesh* mesh = resourceManager.getMesh(meshList[i]);

		if (mesh->getFileName().size() == 0)
			continue;

		meshData[mesh->getName()] = mesh->getName() + ".model";
	}
	root["meshes"] = meshData;

	// saving Textures
	std::vector<std::string> texturesList = resourceManager.getTextureList();
	Json::Value texturesData;
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		FETexture* texture = resourceManager.getTexture(texturesList[i]);
		texturesData[texture->getName()] = texture->getName() + ".FETexture";
	}
	root["textures"] = texturesData;

	// saving Materials
	std::vector<std::string> materialList = resourceManager.getMaterialList();
	Json::Value materialData;
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FEMaterial* material = resourceManager.getMaterial(materialList[i]);

		if (material->albedoMap != nullptr) materialData[material->getName()]["albedoMap"] = material->albedoMap->getName();
		if (material->normalMap != nullptr) materialData[material->getName()]["normalMap"] = material->normalMap->getName();
		if (material->roughtnessMap != nullptr) materialData[material->getName()]["roughtnessMap"] = material->roughtnessMap->getName();
		if (material->metalnessMap != nullptr) materialData[material->getName()]["metalnessMap"] = material->metalnessMap->getName();
		if (material->AOMap != nullptr) materialData[material->getName()]["AOMap"] = material->AOMap->getName();
		if (material->displacementMap != nullptr) materialData[material->getName()]["displacementMap"] = material->displacementMap->getName();
	}
	root["materials"] = materialData;

	// saving Entities
	std::vector<std::string> entityList = scene.getEntityList();
	Json::Value entityData;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = scene.getEntity(entityList[i]);

		entityData[entity->getName()]["mesh"] = entity->mesh->getName();
		entityData[entity->getName()]["material"] = entity->material->getName();
		writeTransformToJSON(entityData[entity->getName()]["transformation"], &entity->transform);
	}
	root["entities"] = entityData;

	// saving Lights
	std::vector<std::string> LightList = scene.getLightsList();
	Json::Value lightData;
	for (size_t i = 0; i < LightList.size(); i++)
	{
		FELight* light = scene.getLight(LightList[i]);

		if (light->getType() == FE_DIRECTIONAL_LIGHT)
		{
			FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);

			lightData[directionalLight->getName()]["type"] = directionalLight->getType();
			lightData[directionalLight->getName()]["intensity"] = directionalLight->getIntensity();
			lightData[directionalLight->getName()]["range"] = 0.0f;
			lightData[directionalLight->getName()]["spotAngle"] = 0.0f;
			lightData[directionalLight->getName()]["spotAngleOuter"] = 0.0f;
			lightData[directionalLight->getName()]["castShadows"] = directionalLight->isCastShadows();
			lightData[directionalLight->getName()]["enabled"] = directionalLight->isLightEnabled();
			lightData[directionalLight->getName()]["color"]["R"] = directionalLight->getColor()[0];
			lightData[directionalLight->getName()]["color"]["G"] = directionalLight->getColor()[1];
			lightData[directionalLight->getName()]["color"]["B"] = directionalLight->getColor()[2];
			writeTransformToJSON(lightData[light->getName()]["transformation"], &directionalLight->transform);
			lightData[directionalLight->getName()]["direction"]["X"] = directionalLight->getDirection()[0];
			lightData[directionalLight->getName()]["direction"]["Y"] = directionalLight->getDirection()[1];
			lightData[directionalLight->getName()]["direction"]["Z"] = directionalLight->getDirection()[2];
		}
		else if (light->getType() == FE_SPOT_LIGHT)
		{
			FESpotLight* spotLight = reinterpret_cast<FESpotLight*>(light);

			lightData[spotLight->getName()]["type"] = spotLight->getType();
			lightData[spotLight->getName()]["intensity"] = spotLight->getIntensity();
			lightData[spotLight->getName()]["range"] = spotLight->getRange();
			lightData[spotLight->getName()]["spotAngle"] = spotLight->getSpotAngle();
			lightData[spotLight->getName()]["spotAngleOuter"] = spotLight->getSpotAngleOuter();
			lightData[spotLight->getName()]["castShadows"] = spotLight->isCastShadows();
			lightData[spotLight->getName()]["enabled"] = spotLight->isLightEnabled();
			lightData[spotLight->getName()]["color"]["R"] = spotLight->getColor()[0];
			lightData[spotLight->getName()]["color"]["G"] = spotLight->getColor()[1];
			lightData[spotLight->getName()]["color"]["B"] = spotLight->getColor()[2];
			writeTransformToJSON(lightData[light->getName()]["transformation"], &spotLight->transform);
			lightData[spotLight->getName()]["direction"]["X"] = spotLight->getDirection()[0];
			lightData[spotLight->getName()]["direction"]["Y"] = spotLight->getDirection()[1];
			lightData[spotLight->getName()]["direction"]["Z"] = spotLight->getDirection()[2];
		}
		else if (light->getType() == FE_POINT_LIGHT)
		{
			FEPointLight* pointLight = reinterpret_cast<FEPointLight*>(light);

			lightData[pointLight->getName()]["type"] = pointLight->getType();
			lightData[pointLight->getName()]["intensity"] = pointLight->getIntensity();
			lightData[pointLight->getName()]["range"] = pointLight->getRange();
			lightData[pointLight->getName()]["spotAngle"] = 0.0f;
			lightData[pointLight->getName()]["spotAngleOuter"] = 0.0f;
			lightData[pointLight->getName()]["castShadows"] = pointLight->isCastShadows();
			lightData[pointLight->getName()]["enabled"] = pointLight->isLightEnabled();
			lightData[pointLight->getName()]["color"]["R"] = pointLight->getColor()[0];
			lightData[pointLight->getName()]["color"]["G"] = pointLight->getColor()[1];
			lightData[pointLight->getName()]["color"]["B"] = pointLight->getColor()[2];
			writeTransformToJSON(lightData[light->getName()]["transformation"], &pointLight->transform);
			lightData[pointLight->getName()]["direction"]["X"] = 0.0f;
			lightData[pointLight->getName()]["direction"]["Y"] = 0.0f;
			lightData[pointLight->getName()]["direction"]["Z"] = 0.0f;
		}
	}
	root["lights"] = lightData;

	// saving Camera settings
	Json::Value cameraData;

	cameraData["position"]["X"] = engine.getCamera()->getPosition()[0];
	cameraData["position"]["Y"] = engine.getCamera()->getPosition()[1];
	cameraData["position"]["Z"] = engine.getCamera()->getPosition()[2];

	cameraData["fov"] = engine.getCamera()->getFov();
	cameraData["nearPlane"] = engine.getCamera()->getNearPlane();
	cameraData["farPlane"] = engine.getCamera()->getFarPlane();

	cameraData["yaw"] = engine.getCamera()->getYaw();
	cameraData["pitch"] = engine.getCamera()->getPitch();
	cameraData["roll"] = engine.getCamera()->getRoll();

	cameraData["aspectRatio"] = engine.getCamera()->getAspectRatio();

	cameraData["gamma"] = engine.getCamera()->getGamma();
	cameraData["exposure"] = engine.getCamera()->getExposure();

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
	FEngine& engine = FEngine::getInstance();
	FEResourceManager& resourceManager = FEResourceManager::getInstance();
	FEScene& scene = FEScene::getInstance();
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
		resourceManager.LoadFEMesh((projectFolder + root["meshes"][meshList[i]].asCString()).c_str(), meshList[i]);
	}

	// loading Textures
	std::vector<Json::String> texturesList = root["textures"].getMemberNames();
	for (size_t i = 0; i < texturesList.size(); i++)
	{
		FETexture* justLoadedTexture = resourceManager.LoadFETexture((projectFolder + root["textures"][texturesList[i]].asCString()).c_str(), texturesList[i]);
		// I have texture name in file but for now I will only use name from scene.txt
		resourceManager.setTextureName(justLoadedTexture, texturesList[i]);
	}

	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = resourceManager.createMaterial(materialsList[i].c_str());
		newMat->shader = new FEShader(FEPhongVS, FEPhongFS);

		std::vector<Json::String> textureList = root["materials"][materialsList[i]].getMemberNames();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			if (textureList[j] == "albedoMap")
				newMat->albedoMap = resourceManager.getTexture(root["materials"][materialsList[i]]["albedoMap"].asCString());

			if (textureList[j] == "normalMap")
				newMat->normalMap = resourceManager.getTexture(root["materials"][materialsList[i]]["normalMap"].asCString());

			if (textureList[j] == "AOMap")
				newMat->AOMap = resourceManager.getTexture(root["materials"][materialsList[i]]["AOMap"].asCString());
		}
	}

	// loading Entities
	std::vector<Json::String> entityList = root["entities"].getMemberNames();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		scene.addEntity(resourceManager.getMesh(root["entities"][entityList[i]]["mesh"].asCString()),
						resourceManager.getMaterial(root["entities"][entityList[i]]["material"].asCString()),
						entityList[i]);

		readTransformToJSON(root["entities"][entityList[i]]["transformation"], &scene.getEntity(entityList[i])->transform);
	}

	// loading Lights
	std::vector<Json::String> lightList = root["lights"].getMemberNames();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		scene.addLight(static_cast<FELightType>(root["lights"][lightList[i]]["type"].asInt()), lightList[i]);
		FELight* light = scene.getLight(lightList[i]);

		light->setIntensity(root["lights"][lightList[i]]["intensity"].asFloat());
		if (light->getType() == FE_SPOT_LIGHT)
		{
			reinterpret_cast<FESpotLight*>(light)->setRange(root["lights"][lightList[i]]["range"].asFloat());
		}
		else if (light->getType() == FE_POINT_LIGHT)
		{
			reinterpret_cast<FEPointLight*>(light)->setRange(root["lights"][lightList[i]]["range"].asFloat());
		}
		
		if (light->getType() == FE_SPOT_LIGHT)
		{
			reinterpret_cast<FESpotLight*>(light)->setSpotAngle(root["lights"][lightList[i]]["spotAngle"].asFloat());
			reinterpret_cast<FESpotLight*>(light)->setSpotAngleOuter(root["lights"][lightList[i]]["spotAngleOuter"].asFloat());
		}
		
		light->setCastShadows(root["lights"][lightList[i]]["castShadows"].asBool());
		light->setLightEnabled(root["lights"][lightList[i]]["enabled"].asBool());

		readTransformToJSON(root["lights"][lightList[i]]["transformation"], &light->transform);

		if (light->getType() == FE_DIRECTIONAL_LIGHT)
		{
			reinterpret_cast<FEDirectionalLight*>(light)->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
																	   root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
																	   root["lights"][lightList[i]]["direction"]["Z"].asFloat()));
		}
		else if (light->getType() == FE_SPOT_LIGHT)
		{
			reinterpret_cast<FESpotLight*>(light)->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
				                                                root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
				                                                root["lights"][lightList[i]]["direction"]["Z"].asFloat()));
		}
		
		light->setColor(glm::vec3(root["lights"][lightList[i]]["color"]["R"].asFloat(),
								  root["lights"][lightList[i]]["color"]["G"].asFloat(),
								  root["lights"][lightList[i]]["color"]["B"].asFloat()));
	}

	// loading Camera settings
	engine.getCamera()->setPosition(glm::vec3(root["camera"]["position"]["X"].asFloat(),
											  root["camera"]["position"]["Y"].asFloat(),
											  root["camera"]["position"]["Z"].asFloat()));

	engine.getCamera()->setFov(root["camera"]["fov"].asFloat());
	engine.getCamera()->setNearPlane(root["camera"]["nearPlane"].asFloat());
	engine.getCamera()->setFarPlane(root["camera"]["farPlane"].asFloat());

	engine.getCamera()->setYaw(root["camera"]["yaw"].asFloat());
	engine.getCamera()->setPitch(root["camera"]["pitch"].asFloat());
	engine.getCamera()->setRoll(root["camera"]["roll"].asFloat());

	engine.getCamera()->setAspectRatio(root["camera"]["aspectRatio"].asFloat());

	engine.getCamera()->setGamma(root["camera"]["gamma"].asFloat());
	engine.getCamera()->setExposure(root["camera"]["exposure"].asFloat());

	sceneFile.close();
}

void FEProject::createDummyScreenshot()
{
	size_t width = FEngine::getInstance().getWindowWidth();
	size_t height = FEngine::getInstance().getWindowHeight();

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

	FEResourceManager::getInstance().saveFETexture((getProjectFolder() + "/projectScreenShot.FETexture").c_str(), pixels, width, height);
	delete[] pixels;
}