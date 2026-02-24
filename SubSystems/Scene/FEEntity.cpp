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

Json::Value FEEntity::ToJson()
{
	Json::Value Root;
	Root["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(this);
	SaveComponents(Root["Components"]);

	return Root;
}

void FEEntity::SaveComponents(Json::Value& Root)
{
	std::vector<FEComponentTypeInfo> List = GetComponentsInfoList();
	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].ToJson != nullptr)
			Root[List[i].Name] = List[i].ToJson(this);
	}
}

void FEEntity::FromJson(Json::Value Root)
{
	FEObjectLoadedData Data = RESOURCE_MANAGER.LoadFEObjectPart(Root["FEObjectData"]);

	// ID and Name should be set before calling this function
	if (Data.ID != GetObjectID())
	{
		LOG.Add("FEEntity::FromJson: ID mismatch!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	if (Data.Name != GetName())
	{
		LOG.Add("FEEntity::FromJson: Name mismatch!", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	Tag = Data.Tag;
	Type = Data.Type;

	// Load components.
	std::vector<Json::String> ComponentsList = Root["Components"].getMemberNames();
	COMPONENTS_TOOL.SortComponentsByLoadingPriority(ComponentsList);

	for (size_t i = 0; i < ComponentsList.size(); i++)
	{
		std::string ComponentName = ComponentsList[i];
		FEComponentTypeInfo* ComponentInfo = COMPONENTS_TOOL.GetComponentInfoByName(ComponentName);

		if (ComponentInfo == nullptr)
		{
			LOG.Add("FEEntity::FromJson: Could not find component info for component: " + ComponentName, "FE_LOG_LOADING", FE_LOG_ERROR);
			continue;
		}

		if (ComponentInfo->FromJson == nullptr)
		{
			LOG.Add("FEEntity::FromJson: Component: " + ComponentName + " does not have FromJson function!", "FE_LOG_LOADING", FE_LOG_ERROR);
			continue;
		}

		ComponentInfo->FromJson(this, Root["Components"][ComponentName]);
	}
}

std::vector<FEComponentTypeInfo> FEEntity::GetComponentsInfoList()
{
	std::vector<FEComponentTypeInfo> Result;

	// Loop through all components types
	for (auto&& CurrentComponent : GetRegistry().storage())
	{
		entt::id_type ComponentID = CurrentComponent.first;
		// Add only components that current entity has
		if (auto& Storage = CurrentComponent.second; Storage.contains(EnTTEntity))
		{
			if (COMPONENTS_TOOL.ComponentIDToInfo.find(ComponentID) != COMPONENTS_TOOL.ComponentIDToInfo.end())
			{
				Result.push_back(COMPONENTS_TOOL.ComponentIDToInfo[ComponentID]);
			}
		}
	}

	return Result;
}

FEScene* FEEntity::GetParentScene()
{
	return ParentScene;
}

bool FEEntity::IsVisible()
{
	FENaiveSceneGraphNode* ThisNode = ParentScene->SceneGraph.GetNodeByEntityID(GetObjectID());
	if (ThisNode != nullptr)
	{
		// Check all parent nodes for visibility, if any of them is invisible, the entity is invisible
		FENaiveSceneGraphNode* ParentNode = ThisNode->GetParent();
		if (ParentNode != nullptr)
		{
			while (ParentNode != nullptr)
			{
				FEEntity* NodeEntity = ParentNode->GetEntity();
				if (NodeEntity == nullptr)
					break;

				if (!NodeEntity->IsVisible())
					return false;

				ParentNode = ParentNode->GetParent();
			}
		}
	}

	if (!HasComponent<FEVisibilityComponent>())
		return true;

	FEVisibilityComponent& VisibilityComponent = GetComponent<FEVisibilityComponent>();
	return VisibilityComponent.IsVisible();
}

void FEEntity::SetVisible(bool Value)
{
	if (!HasComponent<FEVisibilityComponent>())
		AddComponent<FEVisibilityComponent>();

	FEVisibilityComponent& VisibilityComponent = GetComponent<FEVisibilityComponent>();
	VisibilityComponent.SetVisible(Value);
}

bool FEEntity::IsComponentVisible(ComponentVisibilityType Type)
{
	if (Type == ComponentVisibilityType::ALL)
		return IsVisible();

	if (!HasComponent<FEVisibilityComponent>())
		return true;

	FEVisibilityComponent& VisibilityComponent = GetComponent<FEVisibilityComponent>();
	return VisibilityComponent.IsVisible(Type);
}

void FEEntity::SetComponentVisible(ComponentVisibilityType Type, bool Value)
{
	if (Type == ComponentVisibilityType::ALL)
	{
		SetVisible(Value);
		return;
	}

	if (!HasComponent<FEVisibilityComponent>())
		AddComponent<FEVisibilityComponent>();

	FEVisibilityComponent& VisibilityComponent = GetComponent<FEVisibilityComponent>();
	VisibilityComponent.SetVisible(Type, Value);
}

bool FEEntity::AttachTo(FEEntity* Parent, bool bPreserveWorldTransform)
{
	if (Parent == nullptr)
	{
		LOG.Add("Attempted to attach entity to null parent in FEEntity::AttachTo", "FE_SCENE_GRAPH", FE_LOG_WARNING);
		return false;
	}

	if (Parent == this)
	{
		LOG.Add("Attempted to attach entity to itself in FEEntity::AttachTo", "FE_SCENE_GRAPH", FE_LOG_WARNING);
		return false;
	}

	if (ParentScene != Parent->GetParentScene())
	{
		LOG.Add("Cannot attach entities from different scenes in FEEntity::AttachTo", "FE_SCENE_GRAPH", FE_LOG_WARNING);
		return false;
	}

	FENaiveSceneGraphNode* ParentNode = ParentScene->SceneGraph.GetNodeByEntityID(Parent->GetObjectID());
	FENaiveSceneGraphNode* ThisNode = ParentScene->SceneGraph.GetNodeByEntityID(GetObjectID());

	if (ParentNode == nullptr || ThisNode == nullptr)
	{
		LOG.Add("Could not find scene graph nodes for entities in FEEntity::AttachTo", "FE_SCENE_GRAPH", FE_LOG_ERROR);
		return false;
	}

	return ParentScene->SceneGraph.MoveNode(ThisNode->GetObjectID(), ParentNode->GetObjectID(), bPreserveWorldTransform);
}

bool FEEntity::AttachChild(FEEntity* Child, bool bPreserveWorldTransform)
{
	if (Child == nullptr)
	{
		LOG.Add("Attempted to attach null child entity in FEEntity::AttachChild", "FE_SCENE_GRAPH", FE_LOG_WARNING);
		return false;
	}

	return Child->AttachTo(this, bPreserveWorldTransform);
}

bool FEEntity::Detach(bool bPreserveWorldTransform)
{
	if (ParentScene == nullptr)
	{
		LOG.Add("Cannot detach entity with no parent scene in FEEntity::Detach", "FE_SCENE_GRAPH", FE_LOG_WARNING);
		return false;
	}

	FENaiveSceneGraphNode* ThisNode = ParentScene->SceneGraph.GetNodeByEntityID(GetObjectID());
	if (ThisNode == nullptr)
	{
		LOG.Add("Could not find scene graph node for entity in FEEntity::Detach", "FE_SCENE_GRAPH", FE_LOG_ERROR);
		return false;
	}

	// Detaching means moving to the root node
	FENaiveSceneGraphNode* RootNode = ParentScene->SceneGraph.GetRoot();
	return ParentScene->SceneGraph.MoveNode(ThisNode->GetObjectID(), RootNode->GetObjectID(), bPreserveWorldTransform);
}

FEEntity* FEEntity::GetParentEntity() const
{
	if (ParentScene == nullptr)
		return nullptr;

	FENaiveSceneGraphNode* ThisNode = ParentScene->SceneGraph.GetNodeByEntityID(GetObjectID());
	if (ThisNode == nullptr)
		return nullptr;

	FENaiveSceneGraphNode* ParentNode = ThisNode->GetParent();
	if (ParentNode == nullptr || ParentNode == ParentScene->SceneGraph.GetRoot())
		return nullptr;

	return ParentNode->GetEntity();
}

std::vector<FEEntity*> FEEntity::GetChildEntities() const
{
	std::vector<FEEntity*> Result;

	if (ParentScene == nullptr)
		return Result;

	FENaiveSceneGraphNode* ThisNode = ParentScene->SceneGraph.GetNodeByEntityID(GetObjectID());
	if (ThisNode == nullptr)
		return Result;

	std::vector<FENaiveSceneGraphNode*> ChildNodes = ThisNode->GetChildren();
	Result.reserve(ChildNodes.size());

	for (FENaiveSceneGraphNode* ChildNode : ChildNodes)
	{
		if (ChildNode->GetEntity() != nullptr)
			Result.push_back(ChildNode->GetEntity());
	}

	return Result;
}

bool FEEntity::IsChildOf(FEEntity* PotentialParent) const
{
	if (PotentialParent == nullptr)
		return false;

	return GetParentEntity() == PotentialParent;
}

bool FEEntity::IsDescendantOf(FEEntity* PotentialAncestor) const
{
	if (PotentialAncestor == nullptr || ParentScene == nullptr)
		return false;

	FENaiveSceneGraphNode* ThisNode = ParentScene->SceneGraph.GetNodeByEntityID(GetObjectID());
	FENaiveSceneGraphNode* AncestorNode = ParentScene->SceneGraph.GetNodeByEntityID(PotentialAncestor->GetObjectID());

	if (ThisNode == nullptr || AncestorNode == nullptr)
		return false;

	return ParentScene->SceneGraph.IsDescendant(AncestorNode, ThisNode);
}

bool FEEntity::IsAncestorOf(FEEntity* PotentialDescendant)
{
	if (PotentialDescendant == nullptr)
		return false;

	return PotentialDescendant->IsDescendantOf(this);
}