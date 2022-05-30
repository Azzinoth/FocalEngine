#include "FEEditorInternalResources.h"
using namespace FocalEngine;

FEEditorInternalResources* FEEditorInternalResources::_instance = nullptr;
FEEditorInternalResources::FEEditorInternalResources() {}
FEEditorInternalResources::~FEEditorInternalResources() {}

void FEEditorInternalResources::addResourceToInternalEditorList(FEObject* object)
{
	if (object == nullptr)
	{
		LOG.add("object is nullptr in function FEEditorInternalResources::addResourceToInternalEditorList.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return;
	}

	internalEditorObjects[object->getObjectID()] = object;
}

bool FEEditorInternalResources::isInInternalEditorList(FEObject* object)
{
	if (object == nullptr)
	{
		LOG.add("object is nullptr in function FEEditorInternalResources::isInInternalEditorList.", FE_LOG_ERROR, FE_LOG_GENERAL);
		return false;
	}

	return !(internalEditorObjects.find(object->getObjectID()) == internalEditorObjects.end());
}

void FEEditorInternalResources::clearListByType(FEObjectType type)
{
	auto it = internalEditorObjects.begin();
	while (it != internalEditorObjects.end())
	{
		if (it->second->getType() == type)
		{
			auto temp = it->second;
			it++;
			internalEditorObjects.erase(temp->getObjectID());
		}
		else
		{
			it++;
		}
	}
}
