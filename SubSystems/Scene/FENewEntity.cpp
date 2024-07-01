#include "FENewEntity.h"
using namespace FocalEngine;

FENewEntity::FENewEntity(entt::entity AssignedEnTTEntity, FEScene* Scene) : FEObject(FE_OBJECT_TYPE::FE_NEW_ENTITY, "Unnamed NewEntity")
{
	EnTTEntity = AssignedEnTTEntity;
	this->Scene = Scene;
}

FENewEntity::~FENewEntity()
{
	Scene->Registry.destroy(EnTTEntity);
}