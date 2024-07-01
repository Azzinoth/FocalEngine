#include "FENaiveSceneGraphNode.h"
#include "FENewEntity.h"
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
	FETransformComponent& ChildTransform = NodeToWorkOn->GetNewStyleEntity()->GetComponent<FETransformComponent>();
	glm::mat4 ChildWorldMatrix = ChildTransform.GetTransformMatrix();
	glm::mat4 ParentWorldMatrix = GetNewStyleEntity()->GetComponent<FETransformComponent>().GetTransformMatrix();

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
	FETransformComponent& ChildTransform = NodeToWorkOn->GetNewStyleEntity()->GetComponent<FETransformComponent>();

	glm::mat4 ChildWorldMatrix = ChildTransform.GetTransformMatrix();

	// We want preserve the world position of the child
	// First we need to check if current parent is not root node
	if (OldParent != nullptr && OldParent->GetParent() != nullptr)
	{
		// In case it is not root we need to reverce old parent influence.
		//FEEntity* OldParentEntity = reinterpret_cast<FEEntity*>(OldParent->GetOldStyleEntity());
		if (OldParent->Parent != nullptr)
		{
			//FETransformComponent& OldParentTransform = OldParentEntity->Transform;

			glm::mat4 ChildLocalMatrix = ChildTransform.LocalSpaceMatrix;
			glm::mat4 OldParentWorldMatrix = OldParent->GetNewStyleEntity()->GetComponent<FETransformComponent>().GetTransformMatrix();

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

//FENaiveSceneGraphNode* FENaiveSceneGraphNode::GetChildByOldEntityID(std::string NewEntityID)
//{
//	if (OldStyleEntity != nullptr && OldStyleEntity->GetObjectID() == NewEntityID)
//		return this;
//
//	// If we reach here, the child was not found
//	// It is possible that the child is a child of a child
//	// So we need to search recursively
//	for (size_t i = 0; i < Children.size(); i++)
//	{
//		FENaiveSceneGraphNode* Child = Children[i]->GetChildByOldEntityID(NewEntityID);
//		if (Child != nullptr)
//			return Child;
//	}
//
//	return nullptr;
//}

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

std::vector<FENaiveSceneGraphNode*> FENaiveSceneGraphNode::GetRecursiveChildren()
{
	std::vector<FENaiveSceneGraphNode*> Result;
	for (size_t i = 0; i < Children.size(); i++)
	{
		Result.push_back(Children[i]);
		std::vector<FENaiveSceneGraphNode*> SubResult = Children[i]->GetRecursiveChildren();
		for (size_t j = 0; j < SubResult.size(); j++)
			Result.push_back(SubResult[j]);
	}
	return Result;
}

//FEObject* FENaiveSceneGraphNode::GetOldStyleEntity()
//{
//	return OldStyleEntity;
//}

FENewEntity* FENaiveSceneGraphNode::GetNewStyleEntity()
{
	return NewStyleEntity;
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

std::vector<FENaiveSceneGraphNode*> FENaiveSceneGraphNode::GetAllNodesInternal()
{
	std::vector<FENaiveSceneGraphNode*> Result;
	Result.push_back(this);

	for (size_t i = 0; i < Children.size(); i++)
	{
		std::vector<FENaiveSceneGraphNode*> SubResult = Children[i]->GetAllNodesInternal();
		for (size_t j = 0; j < SubResult.size(); j++)
			Result.push_back(SubResult[j]);
	}

	return Result;
}

Json::Value FENaiveSceneGraphNode::ToJson()
{
	Json::Value Node;
	Node["Name"] = GetName();
	Node["ID"] = GetObjectID();
	// Only root node does not have ParentID
	Node["ParentID"] = Parent->GetObjectID();
	Node["NewEntityID"] = NewStyleEntity->GetObjectID();

	Json::Value ChildrenArray;
	for (size_t i = 0; i < Children.size(); i++)
	{
		ChildrenArray[std::to_string(i)]["ID"] = Children[i]->GetObjectID();
	}
	Node["Children"] = ChildrenArray;

	return Node;
}

void FENaiveSceneGraphNode::FromJson(Json::Value Root)
{
	SetName(Root["Name"].asString());
	SetID(Root["ID"].asString());
	std::string NewEntityID = Root["NewEntityID"].asString();

	NewStyleEntity = reinterpret_cast<FENewEntity*>(OBJECT_MANAGER.GetFEObject(NewEntityID));
	if (NewStyleEntity == nullptr)
		LOG.Add("FENaiveSceneGraphNode::FromJson: Could not find entity with ID: " + NewEntityID, "FE_LOG_LOADING", FE_LOG_ERROR);
}

FENaiveSceneGraphNode* FENaiveSceneGraphNode::GetChildByNewEntityID(std::string NewEntityID)
{
	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i]->NewStyleEntity->GetObjectID() == NewEntityID)
			return Children[i];
	}

	// If we reach here, the child was not found
	// It is possible that the child is a child of a child
	// So we need to search recursively
	for (size_t i = 0; i < Children.size(); i++)
	{
		FENaiveSceneGraphNode* Child = Children[i]->GetChildByNewEntityID(NewEntityID);
		if (Child != nullptr)
			return Child;
	}

	return nullptr;
}