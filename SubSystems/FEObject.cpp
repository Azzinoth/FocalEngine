#include "FEObject.h"
using namespace FocalEngine;

FEObjectManager* FEObjectManager::_instance = nullptr;

FEObjectManager::FEObjectManager()
{
	objectsByType.resize(16);
}

FEObjectManager::~FEObjectManager()
{
}

FEObject* FEObjectManager::getFEObject(std::string ID)
{
	if (allObjects.find(ID) != allObjects.end())
		return allObjects[ID];

	return nullptr;
}

FEObject::FEObject(FEObjectType objectType, std::string objectName)
{
	ID = getUniqueHexID();
	//ID = getUniqueId();
	//std::string IDinHex = "";

	//for (size_t i = 0; i < ID.size(); i++)
	//{
	//	IDinHex.push_back("0123456789ABCDEF"[(ID[i] >> 4) & 15]);
	//	IDinHex.push_back("0123456789ABCDEF"[ID[i] & 15]);
	//}

	//std::string additionalRandomness = getUniqueId();
	//std::string additionalString = "";
	//for (size_t i = 0; i < ID.size(); i++)
	//{
	//	additionalString.push_back("0123456789ABCDEF"[(additionalRandomness[i] >> 4) & 15]);
	//	additionalString.push_back("0123456789ABCDEF"[additionalRandomness[i] & 15]);
	//}
	//std::string finalID = "";

	//for (size_t i = 0; i < ID.size() * 2; i++)
	//{
	//	if (rand() % 2 - 1)
	//	{
	//		finalID += IDinHex[i];
	//	}
	//	else
	//	{
	//		finalID += additionalString[i];
	//	}
	//}

	//ID = finalID;

	type = objectType;
	name = objectName;

	FEObjectManager::getInstance().allObjects[ID] = this;
	FEObjectManager::getInstance().objectsByType[type][ID] = this;
}

FEObject::~FEObject()
{
	if (FEObjectManager::getInstance().allObjects.find(ID) == FEObjectManager::getInstance().allObjects.end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().objectsByType[type].find(ID) == FEObjectManager::getInstance().objectsByType[type].end())
	{
		assert(0);
	}

	for (size_t i = 0; i < callListOnDeleteFEObject.size(); i++)
	{
		FEObject* objectToCall = FEObjectManager::getInstance().allObjects[callListOnDeleteFEObject[i]];
		if (objectToCall != nullptr)
			objectToCall->processOnDeleteCallbacks(ID);
	}

	FEObjectManager::getInstance().allObjects.erase(ID);
	FEObjectManager::getInstance().objectsByType[type].erase(ID);
}

std::string FEObject::getObjectID() const
{
	return ID;
}

FEObjectType FEObject::getType() const
{
	return type;
}

bool FEObject::getDirtyFlag()
{
	return dirtyFlag;
}

void FEObject::setDirtyFlag(bool newDirtyFlag)
{
	dirtyFlag = newDirtyFlag;
}

std::string FEObject::getName()
{
	return name;
}

void FEObject::setName(std::string newName)
{
	if (newName.size() == 0)
		return;
	name = newName;
	nameHash = std::hash<std::string>{}(name);
}

int FEObject::getNameHash()
{
	return nameHash;
}

void FEObject::setID(std::string newID)
{
	if (ID == newID)
	{
		LOG.add("FEObject::setID newID is the same as current ID, redundant call", FE_LOG_INFO, FE_LOG_LOADING);
		return;
	}

	if (FEObjectManager::getInstance().allObjects.find(ID) == FEObjectManager::getInstance().allObjects.end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().allObjects.find(newID) != FEObjectManager::getInstance().allObjects.end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().objectsByType[type].find(ID) == FEObjectManager::getInstance().objectsByType[type].end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().objectsByType[type].find(newID) != FEObjectManager::getInstance().objectsByType[type].end())
	{
		assert(0);
	}

	FEObjectManager::getInstance().objectsByType[type].erase(ID);
	FEObjectManager::getInstance().allObjects.erase(ID);
	ID = newID;
	FEObjectManager::getInstance().allObjects[newID] = this;
	FEObjectManager::getInstance().objectsByType[type][newID] = this;
}

void FEObject::setType(FEObjectType newType)
{
	FEObjectManager::getInstance().objectsByType[type].erase(ID);
	type = newType;
	FEObjectManager::getInstance().objectsByType[type][ID] = this;
}

void FEObject::setIDOfUnTyped(std::string newID)
{
	if (type != FE_NULL)
	{
		LOG.add("FEObject::setIDOfUnTyped type is FE_NULL", FE_LOG_WARNING, FE_LOG_LOADING);
		return;
	}

	if (ID == newID)
	{
		LOG.add("FEObject::setIDOfUnTyped newID is the same as current ID, redundant call", FE_LOG_INFO, FE_LOG_LOADING);
		return;
	}

	if (FEObjectManager::getInstance().allObjects.find(ID) == FEObjectManager::getInstance().allObjects.end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().allObjects.find(newID) != FEObjectManager::getInstance().allObjects.end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().objectsByType[type].find(ID) == FEObjectManager::getInstance().objectsByType[type].end())
	{
		assert(0);
	}

	if (FEObjectManager::getInstance().objectsByType[type].find(newID) != FEObjectManager::getInstance().objectsByType[type].end())
	{
		assert(0);
	}

	FEObjectManager::getInstance().objectsByType[type].erase(ID);
	FEObjectManager::getInstance().allObjects.erase(ID);
	ID = newID;
	FEObjectManager::getInstance().allObjects[newID] = this;
	FEObjectManager::getInstance().objectsByType[type][newID] = this;
}

void FEObject::processOnDeleteCallbacks(std::string deletingFEObject)
{

}