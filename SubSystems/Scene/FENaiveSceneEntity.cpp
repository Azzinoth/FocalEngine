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

#include <glm/gtx/matrix_decompose.hpp>
void FENaiveSceneEntity::AddChild(FENaiveSceneEntity* Child)
{
	Children.push_back(Child);
	Child->Parent = this;

	// If we want to add child without changing world position of it.
	bool bPreserveWorldPosition = true;
	if (bPreserveWorldPosition)
	{
		FEEntity* Entity = reinterpret_cast<FEEntity*>(GetOldStyleEntity());
		if (Entity == nullptr)
			return;

		FETransformComponent& ParentTransform = Entity->Transform;

		FEEntity* ChildEntity = reinterpret_cast<FEEntity*>(Child->GetOldStyleEntity());
		FETransformComponent& ChildTransform = ChildEntity->Transform;

		// Get the world matrices
		glm::mat4 ParentWorldMatrix = ParentTransform.GetTransformMatrix();
		glm::mat4 ChildWorldMatrix = ChildTransform.GetTransformMatrix();

		// Calculate the inverse of the parent's world matrix
		glm::mat4 ParentWorldInverseMatrix = glm::inverse(ParentWorldMatrix);

		// Calculate the new local matrix for the child
		glm::mat4 ChildLocalMatrix = ParentWorldInverseMatrix * ChildWorldMatrix;

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(ChildLocalMatrix, scale, rotation, translation, skew, perspective);

		ChildTransform.SetPosition(translation);
		ChildTransform.SetQuaternion(rotation);
		ChildTransform.SetScale(scale);

		/*ChildTransform.SetPosition(ChildTransform.GetPosition() - ParentTransform.GetPosition());
		glm::quat ParentInverseRotation = glm::inverse(ParentTransform.GetQuaternion());
		ChildTransform.SetQuaternion(ParentInverseRotation * ChildTransform.GetQuaternion());
		ChildTransform.SetScale(ChildTransform.GetScale() / ParentTransform.GetScale());*/
	}
}

void FENaiveSceneEntity::RemoveChild(FENaiveSceneEntity* Child)
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i] == Child)
		{
			Child->Parent = nullptr;
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

FENaiveSceneEntity* FENaiveSceneEntity::GetParent()
{
	return Parent;
}