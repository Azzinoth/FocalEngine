#include "FEObject.h"
using namespace FocalEngine;

FEObjectManager* FEObjectManager::_instance = nullptr;

FEObjectManager::FEObjectManager()
{
	objectsByType.resize(17);
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
	ID = APPLICATION.getUniqueHexID();
	
	type = objectType;
	name = objectName;

	OBJECT_MANAGER.allObjects[ID] = this;
	OBJECT_MANAGER.objectsByType[type][ID] = this;
}

FEObject::~FEObject()
{
	if (OBJECT_MANAGER.allObjects.find(ID) == OBJECT_MANAGER.allObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.objectsByType[type].find(ID) == OBJECT_MANAGER.objectsByType[type].end())
	{
		assert(0);
	}

	for (size_t i = 0; i < callListOnDeleteFEObject.size(); i++)
	{
		FEObject* objectToCall = OBJECT_MANAGER.allObjects[callListOnDeleteFEObject[i]];
		if (objectToCall != nullptr)
			objectToCall->processOnDeleteCallbacks(ID);
	}

	OBJECT_MANAGER.allObjects.erase(ID);
	OBJECT_MANAGER.objectsByType[type].erase(ID);
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
	nameHash = int(std::hash<std::string>{}(name));
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

	if (OBJECT_MANAGER.allObjects.find(ID) == OBJECT_MANAGER.allObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.allObjects.find(newID) != OBJECT_MANAGER.allObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.objectsByType[type].find(ID) == OBJECT_MANAGER.objectsByType[type].end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.objectsByType[type].find(newID) != OBJECT_MANAGER.objectsByType[type].end())
	{
		assert(0);
	}

	OBJECT_MANAGER.objectsByType[type].erase(ID);
	OBJECT_MANAGER.allObjects.erase(ID);
	ID = newID;
	OBJECT_MANAGER.allObjects[newID] = this;
	OBJECT_MANAGER.objectsByType[type][newID] = this;
}

void FEObject::setType(FEObjectType newType)
{
	OBJECT_MANAGER.objectsByType[type].erase(ID);
	type = newType;
	OBJECT_MANAGER.objectsByType[type][ID] = this;
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

	if (OBJECT_MANAGER.allObjects.find(ID) == OBJECT_MANAGER.allObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.allObjects.find(newID) != OBJECT_MANAGER.allObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.objectsByType[type].find(ID) == OBJECT_MANAGER.objectsByType[type].end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.objectsByType[type].find(newID) != OBJECT_MANAGER.objectsByType[type].end())
	{
		assert(0);
	}

	OBJECT_MANAGER.objectsByType[type].erase(ID);
	OBJECT_MANAGER.allObjects.erase(ID);
	ID = newID;
	OBJECT_MANAGER.allObjects[newID] = this;
	OBJECT_MANAGER.objectsByType[type][newID] = this;
}

void FEObject::processOnDeleteCallbacks(std::string deletingFEObject)
{

}