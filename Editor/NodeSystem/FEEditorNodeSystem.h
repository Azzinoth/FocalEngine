#pragma once

#include "FEEditorNodeArea.h"

class FEEditorNodeSystem
{
	SINGLETON_PRIVATE_PART(FEEditorNodeSystem)

	std::vector<FEEditorNodeArea*> createdAreas;
public:
	SINGLETON_PUBLIC_PART(FEEditorNodeSystem)

	FEEditorNodeArea* createNodeArea();
	void deleteNodeArea(FEEditorNodeArea* nodeArea);
};

#define NODE_SYSTEM FEEditorNodeSystem::getInstance()