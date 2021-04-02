#include "FEGameModel.h"
using namespace FocalEngine;

FEGameModel::FEGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name) : FEObject(FE_GAMEMODEL, Name)
{
	mesh = Mesh;
	material = Material;
	setName(Name);
	LODRecords.resize(4);

	LODRecords[0].maxDrawDistance = 50.0f;
	LODRecords[1].maxDrawDistance = 150.0f;
	LODRecords[2].maxDrawDistance = 200.0f;
	LODRecords[3].maxDrawDistance = cullDistance;
}

FEGameModel::~FEGameModel()
{
}

bool FEGameModel::useLODlevels()
{
	return haveLODlevels;
}

void FEGameModel::setUsingLODlevels(bool newValue)
{
	if (haveLODlevels == newValue)
		return;
	haveLODlevels = newValue;

	for (size_t i = 1; i < LODRecords.size(); i++)
	{
		LODRecords[i].mesh = nullptr;
	}
}

FEMesh* FEGameModel::getLODMesh(size_t LODindex)
{
	if (LODindex >= LODRecords.size())
		return nullptr;

	if (LODindex == 0 && LODRecords[LODindex].mesh == nullptr)
		return mesh;

	return LODRecords[LODindex].mesh;
}

void FEGameModel::setLODMesh(size_t LODindex, FEMesh* newValue)
{
	if (LODindex >= LODRecords.size())
		return;

	if (LODindex == 0)
		mesh = newValue;

	LODRecords[LODindex].mesh = newValue;
	dirtyFlag = true;
}

size_t FEGameModel::getLODCount()
{
	int realLODCount = 0;
	for (size_t j = 0; j < LODRecords.size(); j++)
	{
		if (getLODMesh(j) == nullptr)
			break;
		realLODCount++;
	}

	return realLODCount;
}

FEMesh* FEGameModel::getMesh()
{
	if (this->haveLODlevels && LODRecords[0].mesh != nullptr)
	{
		return LODRecords[0].mesh;
	}
	else
	{
		return mesh;
	}
}

void FEGameModel::setMesh(FEMesh* newValue)
{
	LODRecords[0].mesh = newValue;
	mesh = newValue;
}

float FEGameModel::getLODMaxDrawDistance(size_t LODindex)
{
	if (LODindex >= LODRecords.size())
		return 0.0f;

	return LODRecords[LODindex].maxDrawDistance;
}

void FEGameModel::setLODMaxDrawDistance(size_t LODindex, float newValue)
{
	if (LODindex >= LODRecords.size())
		return;

	LODRecords[LODindex].maxDrawDistance = newValue;
	correctLODDistances();
}

bool FEGameModel::isLODBillboard(size_t LODindex)
{
	if (LODindex >= LODRecords.size())
		return false;

	return LODRecords[LODindex].isBillboard;
}

void FEGameModel::setIsLODBillboard(size_t LODindex, bool newValue)
{
	if (LODindex >= LODRecords.size())
		return;

	if (newValue && !LODRecords[LODindex].isBillboard)
	{
		for (size_t i = 0; i < LODRecords.size(); i++)
		{
			LODRecords[LODindex].isBillboard = false;
		}

		LODRecords[LODindex].isBillboard = newValue;
		LODRecords[LODindex].maxDrawDistance = cullDistance;

		for (size_t i = LODindex + 1; i < LODRecords.size(); i++)
		{
			LODRecords[i].mesh = nullptr;
			LODRecords[i].maxDrawDistance = cullDistance + 10.0f;
		}
	}

	LODRecords[LODindex].isBillboard = newValue;
}

FEMaterial* FEGameModel::getMaterial()
{
	return material;
}

FEMaterial* FEGameModel::getBillboardMaterial()
{
	return billboardMaterial;
}

void FEGameModel::setMaterial(FEMaterial* newValue)
{
	material = newValue;
}

void FEGameModel::setBillboardMaterial(FEMaterial* newValue)
{
	billboardMaterial = newValue;
}

float FEGameModel::getBillboardZeroRotaion()
{
	return billboardZeroRotaion;
}

void FEGameModel::setBillboardZeroRotaion(float newValue)
{
	billboardZeroRotaion = newValue;
}

float FEGameModel::getCullDistance()
{
	return cullDistance;
}

void FEGameModel::setCullDistance(float newValue)
{
	if (newValue < 0.1f)
		newValue = 0.1f;

	cullDistance = newValue;
	correctLODDistances();
}

size_t FEGameModel::getMaxLODCount()
{
	return maxLODCount;
}

float FEGameModel::getScaleFactor()
{
	return scaleFactor;
}

void FEGameModel::setScaleFactor(float newValue)
{
	scaleFactor = newValue;
}

void FEGameModel::correctLODDistances()
{
	for (size_t i = 0; i < LODRecords.size(); i++)
	{
		if (LODRecords[i].mesh == nullptr)
		{
			LODRecords[i].maxDrawDistance = cullDistance + 1;
		}
	}
}