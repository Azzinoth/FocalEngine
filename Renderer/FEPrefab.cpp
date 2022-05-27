#include "FEPrefab.h"
using namespace FocalEngine;

FEPrefab::FEPrefab() : FEObject(FE_PREFAB, "")
{
	setDirtyFlag(true);
}

FEPrefab::FEPrefab(FEGameModel* gameModel, std::string Name) : FEObject(FE_PREFAB, Name)
{
	components.push_back(new FEPrefabComponent());
	components.back()->gameModel = gameModel;

	setDirtyFlag(true);
}

FEPrefab::~FEPrefab()
{
	for (int i = 0; i < components.size(); i++)
	{
		delete components[i];
	}
}

void FEPrefab::updateAABB()
{
	AABB = FEAABB();

	for (int i = 0; i < components.size(); i++)
	{
		if (components[i]->gameModel == nullptr || components[i]->gameModel->mesh == nullptr)
			continue;
		AABB = AABB.merge(components[i]->gameModel->mesh->getAABB().transform(components[i]->transform.getTransformMatrix()));
	}
}

FEAABB FEPrefab::getAABB()
{
	if (getDirtyFlag())
		updateAABB();

	return AABB;
}

bool FEPrefab::usesMaterial(std::string materialID)
{
	for (int i = 0; i < components.size(); i++)
	{
		if (components[i]->gameModel->material->getObjectID() == materialID)
			return true;

		if (components[i]->gameModel->billboardMaterial != nullptr && components[i]->gameModel->billboardMaterial->getObjectID() == materialID)
			return true;
	}

	return false;
}

bool FEPrefab::usesGameModel(std::string gameModelID)
{
	for (int i = 0; i < components.size(); i++)
	{
		if (components[i]->gameModel->getObjectID() == gameModelID)
			return true;
	}

	return false;
}

int FEPrefab::componentsCount()
{
	return int(components.size());
}

void FEPrefab::addComponent(FEGameModel* gameModel, FETransformComponent transform)
{
	if (gameModel == nullptr)
		return;

	components.push_back(new FEPrefabComponent());
	components.back()->gameModel = gameModel;
	components.back()->transform = transform;

	setDirtyFlag(true);
}

FEPrefabComponent* FEPrefab::getComponent(int index)
{
	if (index >= components.size())
		return nullptr;

	return components[index];
}

void FEPrefab::removeComponent(FEGameModel* gameModel)
{
	for (int i = 0; i < components.size(); i++)
	{
		if (components[i]->gameModel == gameModel)
		{
			removeComponent(i);
			i--;
		}
	}
}

void FEPrefab::removeComponent(int index)
{
	if (index >= components.size())
		return;

	delete components[index];
	components.erase(components.begin() + index);

	setDirtyFlag(true);
}