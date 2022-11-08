#include "FEObject.h"
using namespace FocalEngine;

FEObjectManager* FEObjectManager::Instance = nullptr;

FEObjectManager::FEObjectManager()
{
	ObjectsByType.resize(17);
}

FEObjectManager::~FEObjectManager()
{
}

FEObject* FEObjectManager::GetFEObject(std::string ID)
{
	if (AllObjects.find(ID) != AllObjects.end())
		return AllObjects[ID];

	return nullptr;
}

FEObject::FEObject(const FE_OBJECT_TYPE ObjectType, const std::string ObjectName)
{
	ID = APPLICATION.GetUniqueHexID();
	
	Type = ObjectType;
	Name = ObjectName;

	OBJECT_MANAGER.AllObjects[ID] = this;
	OBJECT_MANAGER.ObjectsByType[Type][ID] = this;
}

FEObject::~FEObject()
{
	if (OBJECT_MANAGER.AllObjects.find(ID) == OBJECT_MANAGER.AllObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.ObjectsByType[Type].find(ID) == OBJECT_MANAGER.ObjectsByType[Type].end())
	{
		assert(0);
	}

	for (size_t i = 0; i < CallListOnDeleteFEObject.size(); i++)
	{
		FEObject* ObjectToCall = OBJECT_MANAGER.AllObjects[CallListOnDeleteFEObject[i]];
		if (ObjectToCall != nullptr)
			ObjectToCall->ProcessOnDeleteCallbacks(ID);
	}

	OBJECT_MANAGER.AllObjects.erase(ID);
	OBJECT_MANAGER.ObjectsByType[Type].erase(ID);
}

std::string FEObject::GetObjectID() const
{
	return ID;
}

FE_OBJECT_TYPE FEObject::GetType() const
{
	return Type;
}

bool FEObject::IsDirty() const
{
	return bDirtyFlag;
}

void FEObject::SetDirtyFlag(const bool NewValue)
{
	bDirtyFlag = NewValue;
}

std::string FEObject::GetName()
{
	return Name;
}

void FEObject::SetName(const std::string NewValue)
{
	if (NewValue.empty())
		return;
	Name = NewValue;
	NameHash = static_cast<int>(std::hash<std::string>{}(Name));
}

int FEObject::GetNameHash() const
{
	return NameHash;
}

void FEObject::SetID(std::string NewValue)
{
	if (ID == NewValue)
	{
		LOG.Add("FEObject::setID newID is the same as current ID, redundant call", "FE_LOG_LOADING", FE_LOG_INFO);
		return;
	}

	if (OBJECT_MANAGER.AllObjects.find(ID) == OBJECT_MANAGER.AllObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.AllObjects.find(NewValue) != OBJECT_MANAGER.AllObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.ObjectsByType[Type].find(ID) == OBJECT_MANAGER.ObjectsByType[Type].end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.ObjectsByType[Type].find(NewValue) != OBJECT_MANAGER.ObjectsByType[Type].end())
	{
		assert(0);
	}

	OBJECT_MANAGER.ObjectsByType[Type].erase(ID);
	OBJECT_MANAGER.AllObjects.erase(ID);
	ID = NewValue;
	OBJECT_MANAGER.AllObjects[NewValue] = this;
	OBJECT_MANAGER.ObjectsByType[Type][NewValue] = this;
}

void FEObject::SetType(const FE_OBJECT_TYPE NewValue)
{
	OBJECT_MANAGER.ObjectsByType[Type].erase(ID);
	Type = NewValue;
	OBJECT_MANAGER.ObjectsByType[Type][ID] = this;
}

void FEObject::SetIDOfUnTyped(const std::string NewValue)
{
	if (Type != FE_NULL)
	{
		LOG.Add("FEObject::setIDOfUnTyped type is FE_NULL", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}

	if (ID == NewValue)
	{
		LOG.Add("FEObject::setIDOfUnTyped newID is the same as current ID, redundant call", "FE_LOG_LOADING", FE_LOG_INFO);
		return;
	}

	if (OBJECT_MANAGER.AllObjects.find(ID) == OBJECT_MANAGER.AllObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.AllObjects.find(NewValue) != OBJECT_MANAGER.AllObjects.end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.ObjectsByType[Type].find(ID) == OBJECT_MANAGER.ObjectsByType[Type].end())
	{
		assert(0);
	}

	if (OBJECT_MANAGER.ObjectsByType[Type].find(NewValue) != OBJECT_MANAGER.ObjectsByType[Type].end())
	{
		assert(0);
	}

	OBJECT_MANAGER.ObjectsByType[Type].erase(ID);
	OBJECT_MANAGER.AllObjects.erase(ID);
	ID = NewValue;
	OBJECT_MANAGER.AllObjects[NewValue] = this;
	OBJECT_MANAGER.ObjectsByType[Type][NewValue] = this;
}

void FEObject::ProcessOnDeleteCallbacks(std::string DeletingFEObject)
{

}