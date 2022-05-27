#include "FEEditorInternalResources.h"
using namespace FocalEngine;

FEEditorInternalResources* FEEditorInternalResources::_instance = nullptr;
FEEditorInternalResources::FEEditorInternalResources() {}
FEEditorInternalResources::~FEEditorInternalResources() {}

bool FEEditorInternalResources::isInInternalEditorList(FEMesh* mesh)
{
	return !(internalEditorMesh.find(int(std::hash<std::string>{}(mesh->getName()))) == internalEditorMesh.end());
}

bool FEEditorInternalResources::isInInternalEditorList(FEGameModel* gameModel)
{
	return !(internalEditorGameModels.find(int(std::hash<std::string>{}(gameModel->getName()))) == internalEditorGameModels.end());
}

bool FEEditorInternalResources::isInInternalEditorList(FEEntity* entity)
{
	return !(internalEditorEntities.find(entity->getNameHash()) == internalEditorEntities.end());
}