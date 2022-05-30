#pragma once

#include "FEDearImguiWrapper/FEDearImguiWrapper.h"

class FEEditorInternalResources
{
public:
	SINGLETON_PUBLIC_PART(FEEditorInternalResources)
	SINGLETON_PRIVATE_PART(FEEditorInternalResources)

	std::unordered_map<std::string, FEObject*> internalEditorObjects;

	void addResourceToInternalEditorList(FEObject* object);
	bool isInInternalEditorList(FEObject* object);

	void clearListByType(FEObjectType type);
};

#define EDITOR_INTERNAL_RESOURCES FEEditorInternalResources::getInstance()