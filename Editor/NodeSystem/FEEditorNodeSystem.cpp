#include "FEEditorNodeSystem.h"
using namespace FocalEngine;

FEEditorNodeSystem* FEEditorNodeSystem::_instance = nullptr;

FEEditorNodeSystem::FEEditorNodeSystem() {}
FEEditorNodeSystem::~FEEditorNodeSystem() {}

FEEditorNodeArea* FEEditorNodeSystem::createNodeArea()
{
	createdAreas.push_back(new FEEditorNodeArea());
	return createdAreas.back();
}

void FEEditorNodeSystem::deleteNodeArea(FEEditorNodeArea* nodeArea)
{
	for (size_t i = 0; i < createdAreas.size(); i++)
	{
		if (createdAreas[i] == nodeArea)
		{
			delete createdAreas[i];
			createdAreas.erase(createdAreas.begin() + i, createdAreas.begin() + i + 1);
			return;
		}
	}
}