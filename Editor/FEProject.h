#pragma once

#include "../FEngine.h"

using namespace FocalEngine;

class FEProject
{
public:
	FEProject(std::string Name, std::string ProjectFolder);
	~FEProject();

	std::string getName();
	void setName(std::string newName);

	std::string getProjectFolder();
	void saveScene();
	void loadScene();
private:
	std::string name;
	std::string projectFolder;

	FETexture* sceneScreenShoot;

	void writeTransformToJSON(Json::Value& root, FETransformComponent* transform);
	void readTransformToJSON(Json::Value& root, FETransformComponent* transform);
};