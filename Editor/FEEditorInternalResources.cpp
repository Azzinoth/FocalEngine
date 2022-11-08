#include "FEEditorInternalResources.h"
using namespace FocalEngine;

FEEditorInternalResources* FEEditorInternalResources::Instance = nullptr;
FEEditorInternalResources::FEEditorInternalResources() {}
FEEditorInternalResources::~FEEditorInternalResources() {}

void FEEditorInternalResources::AddResourceToInternalEditorList(FEObject* Object)
{
	if (Object == nullptr)
	{
		LOG.Add("object is nullptr in function FEEditorInternalResources::addResourceToInternalEditorList.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return;
	}

	InternalEditorObjects[Object->GetObjectID()] = Object;
}

bool FEEditorInternalResources::IsInInternalEditorList(const FEObject* Object)
{
	if (Object == nullptr)
	{
		LOG.Add("object is nullptr in function FEEditorInternalResources::isInInternalEditorList.", "FE_LOG_GENERAL", FE_LOG_ERROR);
		return false;
	}

	return !(InternalEditorObjects.find(Object->GetObjectID()) == InternalEditorObjects.end());
}

void FEEditorInternalResources::ClearListByType(const FE_OBJECT_TYPE Type)
{
	auto it = InternalEditorObjects.begin();
	while (it != InternalEditorObjects.end())
	{
		if (it->second->GetType() == Type)
		{
			auto temp = it->second;
			it++;
			InternalEditorObjects.erase(temp->GetObjectID());
		}
		else
		{
			it++;
		}
	}
}
