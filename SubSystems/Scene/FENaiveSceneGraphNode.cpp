#include "FENaiveSceneGraphNode.h"
using namespace FocalEngine;

FENaiveSceneGraphNode::FENaiveSceneGraphNode(std::string Name) : FEObject(FE_SCENE_ENTITY, Name) {}
FENaiveSceneGraphNode::~FENaiveSceneGraphNode()
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		delete Children[i];
	}
	Children.clear();
}

void FENaiveSceneGraphNode::ApplyTransformHierarchy(FENaiveSceneGraphNode* NodeToWorkOn)
{
	FEEntity* ChildEntity = reinterpret_cast<FEEntity*>(NodeToWorkOn->GetOldStyleEntity());
	FETransformComponent& ChildTransform = ChildEntity->Transform;
	glm::mat4 ChildWorldMatrix = ChildTransform.GetTransformMatrix();

	FEEntity* Entity = reinterpret_cast<FEEntity*>(GetOldStyleEntity());
	if (Entity != nullptr)
	{
		FETransformComponent& ParentTransform = Entity->Transform;

		glm::mat4 ParentWorldMatrix = ParentTransform.GetTransformMatrix();

		// Calculate the inverse of the parent's world matrix
		glm::mat4 ParentWorldInverseMatrix = glm::inverse(ParentWorldMatrix);

		// Calculate the new local matrix for the child
		glm::mat4 ChildLocalMatrix = ParentWorldInverseMatrix * ChildWorldMatrix;

		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotation;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildLocalMatrix, DoubleTranslation, DoubleRotation, DoubleScale))
		{
			ChildTransform.SetPosition(DoubleTranslation);
			ChildTransform.SetQuaternion(DoubleRotation);
			ChildTransform.SetScale(DoubleScale);
		}
	}
}

void FENaiveSceneGraphNode::AddChild(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform)
{
	if (bPreserveWorldTransform)
		ApplyTransformHierarchy(NodeToAdd);

	Children.push_back(NodeToAdd);
	NodeToAdd->Parent = this;
}

void FENaiveSceneGraphNode::ReverseTransformHierarchy(FENaiveSceneGraphNode* NodeToWorkOn)
{
	FENaiveSceneGraphNode* OldParent = NodeToWorkOn->Parent;

	FEEntity* ChildEntity = reinterpret_cast<FEEntity*>(NodeToWorkOn->GetOldStyleEntity());
	FETransformComponent& ChildTransform = ChildEntity->Transform;
	glm::mat4 ChildWorldMatrix = ChildTransform.GetTransformMatrix();

	// We want preserve the world position of the child
	// First we need to check if current parent is not root node
	if (OldParent != nullptr && OldParent->GetParent() != nullptr)
	{
		// In case it is not root we need to reverce old parent influence.
		FEEntity* OldParentEntity = reinterpret_cast<FEEntity*>(OldParent->GetOldStyleEntity());
		if (OldParentEntity != nullptr)
		{
			FETransformComponent& OldParentTransform = OldParentEntity->Transform;

			glm::mat4 ChildLocalMatrix = ChildTransform.LocalSpaceMatrix;
			glm::mat4 OldParentWorldMatrix = OldParentTransform.GetTransformMatrix();

			// Calculate the child's world matrix
			ChildWorldMatrix = OldParentWorldMatrix * ChildLocalMatrix;

			glm::dvec3 DoubleScale;
			glm::dquat DoubleRotation;
			glm::dvec3 DoubleTranslation;
			if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildWorldMatrix, DoubleTranslation, DoubleRotation, DoubleScale))
			{
				ChildTransform.SetPosition(DoubleTranslation);
				ChildTransform.SetQuaternion(DoubleRotation);
				ChildTransform.SetScale(DoubleScale);
			}
		}
	}
}

void FENaiveSceneGraphNode::DetachChild(FENaiveSceneGraphNode* Child, bool bPreserveWorldTransform)
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i] == Child)
		{
			if (bPreserveWorldTransform)
				ReverseTransformHierarchy(Child);
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
		Children[i]->DetachChild(Child, bPreserveWorldTransform);
	}
}

FENaiveSceneGraphNode* FENaiveSceneGraphNode::GetChild(std::string ID)
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
		FENaiveSceneGraphNode* Child = Children[i]->GetChild(ID);
		if (Child != nullptr)
			return Child;
	}

	return nullptr;
}

FENaiveSceneGraphNode* FENaiveSceneGraphNode::GetChildByOldEntityID(std::string OldEntityID)
{
	if (OldStyleEntity != nullptr && OldStyleEntity->GetObjectID() == OldEntityID)
		return this;

	// If we reach here, the child was not found
	// It is possible that the child is a child of a child
	// So we need to search recursively
	for (size_t i = 0; i < Children.size(); i++)
	{
		FENaiveSceneGraphNode* Child = Children[i]->GetChildByOldEntityID(OldEntityID);
		if (Child != nullptr)
			return Child;
	}

	return nullptr;
}

std::vector<FENaiveSceneGraphNode*> FENaiveSceneGraphNode::GetChildByName(std::string Name)
{
	std::vector<FENaiveSceneGraphNode*> Result;

	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i]->GetName() == Name)
			Result.push_back(Children[i]);
	}

	for (size_t i = 0; i < Children.size(); i++)
	{
		std::vector<FENaiveSceneGraphNode*> SubResult = Children[i]->GetChildByName(Name);
		for (size_t j = 0; j < SubResult.size(); j++)
			Result.push_back(SubResult[j]);
	}

	return Result;
}

std::vector<FENaiveSceneGraphNode*> FENaiveSceneGraphNode::GetChildren()
{
	return Children;
}

FEObject* FENaiveSceneGraphNode::GetOldStyleEntity()
{
	return OldStyleEntity;
}

FENaiveSceneGraphNode* FENaiveSceneGraphNode::GetParent()
{
	return Parent;
}

size_t FENaiveSceneGraphNode::GetImediateChildrenCount()
{
	return Children.size();
}

size_t FENaiveSceneGraphNode::GetRecursiveChildCount()
{
	size_t Count = Children.size();
	for (size_t i = 0; i < Children.size(); i++)
	{
		Count += Children[i]->GetRecursiveChildCount();
	}
	return Count;
}