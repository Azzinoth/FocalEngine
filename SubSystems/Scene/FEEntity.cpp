#include "FEEntity.h"
using namespace FocalEngine;
#include "FEScene.h"

FEEntity::FEEntity(entt::entity AssignedEnTTEntity, FEScene* Scene) : FEObject(FE_OBJECT_TYPE::FE_NEW_ENTITY, "Unnamed Entity")
{
	EnTTEntity = AssignedEnTTEntity;
	this->Scene = Scene;
}

entt::registry& FEEntity::GetRegistry()
{
	return Scene->Registry;
}

FEEntity::~FEEntity()
{
	Scene->Registry.destroy(EnTTEntity);
}