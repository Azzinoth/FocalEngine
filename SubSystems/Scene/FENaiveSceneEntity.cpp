#include "FENaiveSceneEntity.h"
using namespace FocalEngine;

FENaiveSceneEntity::FENaiveSceneEntity(std::string Name) : FEObject(FE_SCENE_ENTITY, Name) {}
FENaiveSceneEntity::~FENaiveSceneEntity()
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		delete Children[i];
	}
	Children.clear();
}

void FENaiveSceneEntity::AddChild(FENaiveSceneEntity* Child)
{
	Children.push_back(Child);

	bool bPreserveWorldPosition = false;
	if (bPreserveWorldPosition)
	{
		// If I want to have to add child without changing world position of it.
		FEEntity* Entity = reinterpret_cast<FEEntity*>(GetOldStyleEntity());
		if (Entity == nullptr)
			return;

		FETransformComponent& ParentTransform = Entity->Transform;

		FEEntity* ChildEntity = reinterpret_cast<FEEntity*>(Child->GetOldStyleEntity());
		FETransformComponent& ChildTransform = ChildEntity->Transform;

		ChildTransform.SetPosition(ChildTransform.GetPosition() - ParentTransform.GetPosition());
		//ChildTransform.SetQuaternion(ChildTransform.GetQuaternion() - ParentTransform.GetQuaternion());
		ChildTransform.SetScale(ChildTransform.GetScale() / ParentTransform.GetScale());
	}
}

void FENaiveSceneEntity::RemoveChild(FENaiveSceneEntity* Child)
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i] == Child)
		{
			Children.erase(Children.begin() + i);
			return;
		}
	}

	// If we reach here, the child was not found
	// It is possible that the child is a child of a child
	// So we need to search recursively
	for (size_t i = 0; i < Children.size(); i++)
	{
		Children[i]->RemoveChild(Child);
	}
}

FENaiveSceneEntity* FENaiveSceneEntity::GetChild(std::string ID)
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i]->GetObjectID() == ID)
			return Children[i];
	}

	// If we reach here, the child was not found
	// It is possible that the child is a child of a child
	// So we need to search recursively
	for (size_t i = 0; i < Children.size(); i++)
	{
		FENaiveSceneEntity* Child = Children[i]->GetChild(ID);
		if (Child != nullptr)
			return Child;
	}

	return nullptr;
}

std::vector<FENaiveSceneEntity*> FENaiveSceneEntity::GetChildByName(std::string Name)
{
	std::vector<FENaiveSceneEntity*> Result;

	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i]->GetName() == Name)
			Result.push_back(Children[i]);
	}

	for (size_t i = 0; i < Children.size(); i++)
	{
		std::vector<FENaiveSceneEntity*> SubResult = Children[i]->GetChildByName(Name);
		for (size_t j = 0; j < SubResult.size(); j++)
			Result.push_back(SubResult[j]);
	}

	return Result;
}

std::vector<FENaiveSceneEntity*> FENaiveSceneEntity::GetChildren()
{
	return Children;
}

FEObject* FENaiveSceneEntity::GetOldStyleEntity()
{
	return OldStyleEntity;
}