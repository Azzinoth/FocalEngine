#include "FEObject.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetObjectManager()
{
	return FEObjectManager::GetInstancePointer();
}
#endif

FEObjectManager::FEObjectManager()
{
	ObjectsByType.resize(13);
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

std::string FEObject::GetName() const
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

std::string FEObject::GetTag() const
{
	return Tag;
}

void FEObject::SetTag(std::string NewValue)
{
	Tag = NewValue;
}

void FEObjectManager::SaveFEObjectPart(std::fstream& OpenedFile, FEObject* Object)
{
	if (Object == nullptr)
	{
		LOG.Add("FEObjectManager::SaveFEObjectPart Object is nullptr", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	if (!OpenedFile.is_open())
	{
		LOG.Add("FEObjectManager::SaveFEObjectPart file is not open", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	int ObjectIDSize = static_cast<int>(Object->GetObjectID().size() + 1);
	OpenedFile.write((char*)&ObjectIDSize, sizeof(int));
	OpenedFile.write((char*)Object->GetObjectID().c_str(), sizeof(char) * ObjectIDSize);

	FE_OBJECT_TYPE ObjectType = Object->GetType();
	OpenedFile.write((char*)&ObjectType, sizeof(FE_OBJECT_TYPE));

	int TagSize = static_cast<int>(Object->GetTag().size() + 1);
	OpenedFile.write((char*)&TagSize, sizeof(int));
	OpenedFile.write((char*)Object->GetTag().c_str(), sizeof(char) * TagSize);

	int NameSize = static_cast<int>(Object->GetName().size() + 1);
	OpenedFile.write((char*)&NameSize, sizeof(int));
	OpenedFile.write((char*)Object->GetName().c_str(), sizeof(char) * NameSize);
}

FEObjectLoadedData FEObjectManager::LoadFEObjectPart(std::fstream& OpenedFile)
{
	FEObjectLoadedData Result;

	if (!OpenedFile.is_open())
	{
		LOG.Add("FEObjectManager::LoadFEObjectPart file is not open", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	int ObjectIDSize = 0;
	OpenedFile.read((char*)&ObjectIDSize, sizeof(int));
	char* ObjectID = new char[ObjectIDSize];
	OpenedFile.read(ObjectID, sizeof(char) * ObjectIDSize);
	Result.ID = ObjectID;
	delete[] ObjectID;

	FE_OBJECT_TYPE ObjectType;
	OpenedFile.read((char*)&ObjectType, sizeof(FE_OBJECT_TYPE));
	Result.Type = ObjectType;

	int TagSize = 0;
	OpenedFile.read((char*)&TagSize, sizeof(int));
	char* Tag = new char[TagSize];
	OpenedFile.read(Tag, sizeof(char) * TagSize);
	Result.Tag = Tag;
	delete[] Tag;

	int NameSize = 0;
	OpenedFile.read((char*)&NameSize, sizeof(int));
	char* Name = new char[NameSize];
	OpenedFile.read(Name, sizeof(char) * NameSize);
	Result.Name = Name;
	delete[] Name;

	return Result;
}

FEObjectLoadedData FEObjectManager::LoadFEObjectPart(char* FileData, int& CurrentShift)
{
	FEObjectLoadedData Result;

	if (FileData == nullptr)
	{
		LOG.Add("FEObjectManager::LoadFEObjectPart FileData is nullptr", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	int IDSize = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;

	Result.ID.clear();
	Result.ID.reserve(IDSize);
	Result.ID.assign((char*)(&FileData[CurrentShift]), IDSize);
	if (Result.ID[Result.ID.size() - 1] == '\0')
		Result.ID.erase(Result.ID.size() - 1);
	CurrentShift += IDSize;

	Result.Type = FE_OBJECT_TYPE (*(int*)(&FileData[CurrentShift]));
	CurrentShift += 4;

	int TagSize = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;

	Result.Tag.clear();
	Result.Tag.reserve(TagSize);
	Result.Tag.assign((char*)(&FileData[CurrentShift]), TagSize);
	if (Result.Tag[Result.Tag.size() - 1] == '\0')
		Result.Tag.erase(Result.Tag.size() - 1);
	CurrentShift += TagSize;

	int NameSize = *(int*)(&FileData[CurrentShift]);
	CurrentShift += 4;
	
	Result.Name.clear();
	Result.Name.reserve(NameSize);
	Result.Name.assign((char*)(&FileData[CurrentShift]), NameSize);
	if (Result.Name[Result.Name.size() - 1] == '\0')
		Result.Name.erase(Result.Name.size() - 1);
	CurrentShift += NameSize;

	return Result;
}