#include "FEScene.h"
using namespace FocalEngine;

FEScene* FEScene::_instance = nullptr;

FEScene::FEScene()
{
}

void FEScene::add(FEEntity* newEntity)
{
	entityMap[std::to_string(entityMap.size())] = newEntity;
}

void FEScene::add(FELight* newLight)
{
	sceneLights.push_back(newLight);
}