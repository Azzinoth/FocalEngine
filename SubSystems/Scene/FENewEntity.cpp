#include "FENewEntity.h"
using namespace FocalEngine;
#include "FEScene.h"

FENewEntity::FENewEntity(entt::entity AssignedEnTTEntity, FEScene* Scene) : FEObject(FE_OBJECT_TYPE::FE_NEW_ENTITY, "Unnamed NewEntity")
{
	EnTTEntity = AssignedEnTTEntity;
	this->Scene = Scene;
}

entt::registry& FENewEntity::GetRegistry()
{
	return Scene->Registry;
}

FENewEntity::~FENewEntity()
{
	Scene->Registry.destroy(EnTTEntity);
}