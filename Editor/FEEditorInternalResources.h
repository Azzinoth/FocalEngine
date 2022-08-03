#pragma once

#include "FEDearImguiWrapper/FEDearImguiWrapper.h"

class FEEditorInternalResources
{
public:
	SINGLETON_PUBLIC_PART(FEEditorInternalResources)
	SINGLETON_PRIVATE_PART(FEEditorInternalResources)

	std::unordered_map<std::string, FEObject*> InternalEditorObjects;

	void AddResourceToInternalEditorList(FEObject* Object);
	bool IsInInternalEditorList(const FEObject* Object);

	void ClearListByType(FE_OBJECT_TYPE Type);
};

#define EDITOR_INTERNAL_RESOURCES FEEditorInternalResources::getInstance()