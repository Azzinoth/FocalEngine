#include "FEEntity.h"
using namespace FocalEngine;
#include "FEScene.h"

FEEntity::FEEntity(entt::entity AssignedEnTTEntity, FEScene* Scene) : FEObject(FE_OBJECT_TYPE::FE_ENTITY, "Unnamed Entity")
{
	EnTTEntity = AssignedEnTTEntity;
	ParentScene = Scene;
}

entt::registry& FEEntity::GetRegistry()
{
	return ParentScene->Registry;
}

FEEntity::~FEEntity()
{
	ParentScene->Registry.destroy(EnTTEntity);
	ParentScene->ClearEntityRecords(GetObjectID(), EnTTEntity);
}