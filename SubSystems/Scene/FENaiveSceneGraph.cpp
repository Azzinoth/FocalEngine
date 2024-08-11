#include "FENaiveSceneGraph.h"
using namespace FocalEngine;

#include "FEScene.h"

FENaiveSceneGraph::FENaiveSceneGraph()
{
	
}

void FENaiveSceneGraph::Clear()
{
	bClearing = true;
	DeleteNode(Root);
	Initialize(ParentScene);
	bClearing = false;
}

void FENaiveSceneGraph::Initialize(FEScene* Scene)
{
	ParentScene = Scene;
	FENaiveSceneGraphNode* NewRoot = new FENaiveSceneGraphNode("SceneRoot");
	Root = NewRoot;
}

FENaiveSceneGraph::~FENaiveSceneGraph()
{
	delete Root;
}

FENaiveSceneGraphNode* FENaiveSceneGraph::GetRoot() const
{
	return Root;
}

FENaiveSceneGraphNode* FENaiveSceneGraph::GetNode(std::string ID)
{
	if (ID == Root->GetObjectID())
		return Root;

	return Root->GetChild(ID);
}

FENaiveSceneGraphNode* FENaiveSceneGraph::GetNodeByEntityID(std::string EntityID)
{
	return Root->GetChildByEntityID(EntityID);
}

std::string FENaiveSceneGraph::AddNode(FEEntity* Entity, bool bPreserveWorldTransform)
{
	FENaiveSceneGraphNode* NewNode = nullptr;
	NewNode = GetNodeByEntityID(Entity->GetObjectID());
	if (NewNode != nullptr)
	{
		LOG.Add("Entity already exists in the scene graph", "FE_LOG_SCENE", FE_LOG_WARNING);
		// Entity already exists in the scene graph
		return NewNode->GetObjectID();
	}

	NewNode = new FENaiveSceneGraphNode(Entity->GetName());
	NewNode->Entity = Entity;
	Root->AddChild(NewNode, bPreserveWorldTransform);

	return NewNode->GetObjectID();
}

bool FENaiveSceneGraph::MoveNode(std::string NodeID, std::string NewParentID, bool bPreserveWorldTransform)
{
	FENaiveSceneGraphNode* NodeToMove = GetNode(NodeID);
	FENaiveSceneGraphNode* NewParent = GetNode(NewParentID);

	if (NodeToMove == nullptr || NewParent == nullptr)
		return false;

	if (NodeToMove->GetParent() == NewParent)
		return false;

	if (IsDescendant(NodeToMove, NewParent))
		return false;

	FENaiveSceneGraphNode* OldParent = NodeToMove->GetParent();

	// Temporarily detach the node.
	// With temporary detach, bPreserveWorldTransform could be a problem.
	DetachNode(NodeToMove, bPreserveWorldTransform);
	AddNodeInternal(NewParent, NodeToMove, bPreserveWorldTransform);

	// Check for cycles
	if (HasCycle(GetRoot()))
	{
		// If a cycle is created, revert the change
		DetachNode(NodeToMove, bPreserveWorldTransform);
		AddNodeInternal(OldParent, NodeToMove, bPreserveWorldTransform);
		return false;
	}

	return true;
}

FENaiveSceneGraphNode* FENaiveSceneGraph::DuplicateNode(std::string NodeIDToDuplicate, std::string NewParentID, bool bAddCopyInName)
{
	FENaiveSceneGraphNode* NodeToDuplicate = GetNode(NodeIDToDuplicate);
	FENaiveSceneGraphNode* NewParent = GetNode(NewParentID);

	if (NodeToDuplicate == nullptr || NewParent == nullptr)
		return nullptr;

	return DuplicateNode(NodeToDuplicate, NewParent, bAddCopyInName);
}

FENaiveSceneGraphNode* FENaiveSceneGraph::DuplicateNode(FENaiveSceneGraphNode* NodeToDuplicate, FENaiveSceneGraphNode* NewParent, bool bAddCopyInName)
{
	if (NodeToDuplicate == nullptr || NewParent == nullptr)
		return nullptr;

	FENaiveSceneGraphNode* OriginalParent = NodeToDuplicate->GetParent();

	// Try to duplicate the node.
	bool bDuplicationSuccess = true;
	FENaiveSceneGraphNode* TopMostDuplicate = new FENaiveSceneGraphNode(NodeToDuplicate->GetName() + (bAddCopyInName ? "_Copy" : ""));
	TopMostDuplicate->Entity = ParentScene->DuplicateEntity(NodeToDuplicate->Entity, bAddCopyInName ? "" : NodeToDuplicate->Entity->GetName());
	NewParent->AddChild(TopMostDuplicate);

	for (size_t i = 0; i < NodeToDuplicate->Children.size(); i++)
	{
		if (!DuplicateNodeInternal(TopMostDuplicate, NodeToDuplicate->GetChildren()[i], bAddCopyInName))
		{
			bDuplicationSuccess = false;
			break;
		}
	}

	if (!bDuplicationSuccess)
	{
		DeleteNode(TopMostDuplicate);
		return nullptr;
	}

	// Check for cycles
	if (HasCycle(GetRoot()))
	{
		// If a cycle is created, delete duplicated node
		DeleteNode(TopMostDuplicate);
		return nullptr;
	}

	return TopMostDuplicate;
}

bool FENaiveSceneGraph::DuplicateNodeInternal(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToDuplicate, bool bAddCopyInName)
{
	FENaiveSceneGraphNode* Duplicate = new FENaiveSceneGraphNode(NodeToDuplicate->GetName() + (bAddCopyInName ? "_Copy" : ""));
	Duplicate->Entity = ParentScene->DuplicateEntity(NodeToDuplicate->Entity);
	Parent->AddChild(Duplicate);

	for (size_t i = 0; i < NodeToDuplicate->Children.size(); i++)
	{
		if (!DuplicateNodeInternal(Duplicate, NodeToDuplicate->GetChildren()[i], bAddCopyInName))
			return false;
	}

	return true;
}

void FENaiveSceneGraph::AddNodeInternal(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform)
{
	Root->AddChild(NodeToAdd, bPreserveWorldTransform);
}

void FENaiveSceneGraph::AddNodeInternal(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform)
{
	Parent->AddChild(NodeToAdd, bPreserveWorldTransform);
}

FENaiveSceneGraphNode* FENaiveSceneGraph::ImportNode(FENaiveSceneGraphNode* NodeFromDifferentSceneGraph, FENaiveSceneGraphNode* TargetParent, std::function<bool(FEEntity*)> Filter)
{
	FENaiveSceneGraphNode* Result = nullptr;
	if (NodeFromDifferentSceneGraph == nullptr)
	{
		LOG.Add("NodeFromDifferentSceneGraph is nullptr in FENaiveSceneGraph::ImportEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}
	
	FEEntity* EntityFromDifferentScene = NodeFromDifferentSceneGraph->Entity;
	if (EntityFromDifferentScene->GetParentScene() == ParentScene)
	{
		LOG.Add("EntityFromDifferentScene is already in this scene in FENaiveSceneGraph::ImportEntity", "FE_LOG_ECS", FE_LOG_WARNING);
		return Result;
	}

	if (Filter != nullptr && !Filter(EntityFromDifferentScene))
		return Result;

	if (TargetParent == nullptr)
		TargetParent = GetRoot();

	Result = DuplicateNode(NodeFromDifferentSceneGraph, TargetParent, false);

	return Result;
}

void FENaiveSceneGraph::DeleteNode(FENaiveSceneGraphNode* NodeToDelete)
{
	if (NodeToDelete == nullptr)
		return;

	if (NodeToDelete == Root && !bClearing)
		return;

	DetachNode(NodeToDelete);
	delete NodeToDelete;
}

void FENaiveSceneGraph::DetachNode(FENaiveSceneGraphNode* NodeToDetach, bool bPreserveWorldTransform)
{
	if (NodeToDetach == Root && !bClearing)
		return;

	Root->DetachChild(NodeToDetach, bPreserveWorldTransform);
}

std::vector<FENaiveSceneGraphNode*> FENaiveSceneGraph::GetNodeByName(std::string Name)
{
	std::vector<FENaiveSceneGraphNode*> Entities;
	std::vector<FENaiveSceneGraphNode*> Children = Root->GetChildren();

	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i]->GetName() == Name)
			Entities.push_back(Children[i]);
	}

	return Entities;
}

bool FENaiveSceneGraph::IsDescendant(FENaiveSceneGraphNode* PotentialAncestor, FENaiveSceneGraphNode* PotentialDescendant)
{
	if (PotentialDescendant == nullptr)
		return false;

	if (PotentialAncestor == PotentialDescendant)
		return true;

	return IsDescendant(PotentialAncestor, PotentialDescendant->GetParent());
}

bool FENaiveSceneGraph::HasCycle(FENaiveSceneGraphNode* NodeToCheck)
{
	std::unordered_set<FENaiveSceneGraphNode*> Visited;
	std::unordered_set<FENaiveSceneGraphNode*> RecursionStack;
	return HasCycleInternal(NodeToCheck, Visited, RecursionStack);
}

bool FENaiveSceneGraph::HasCycleInternal(FENaiveSceneGraphNode* NodeToCheck,
										 std::unordered_set<FENaiveSceneGraphNode*>& Visited,
										 std::unordered_set<FENaiveSceneGraphNode*>& RecursionStack)
{
	if (NodeToCheck == nullptr)
		return false;

	// If the node is already in the recursion stack, we've found a cycle
	if (RecursionStack.find(NodeToCheck) != RecursionStack.end())
		return true;

	// If we've already visited this node and found no cycles, we can return false
	if (Visited.find(NodeToCheck) != Visited.end())
		return false;

	// Mark the current node as visited and part of recursion stack
	Visited.insert(NodeToCheck);
	RecursionStack.insert(NodeToCheck);

	// Recur for all the children
	for (auto& child : NodeToCheck->GetChildren())
	{
		if (HasCycleInternal(child, Visited, RecursionStack))
			return true;
	}

	// Remove the node from recursion stack
	RecursionStack.erase(NodeToCheck);

	return false;
}

size_t FENaiveSceneGraph::GetNodeCount()
{
	return Root->GetRecursiveChildCount();
}

std::vector<FENaiveSceneGraphNode*> FENaiveSceneGraph::GetAllNodes()
{
	return Root->GetAllNodesInternal();
}

Json::Value FENaiveSceneGraph::ToJson(std::function<bool(FEEntity*)> Filter)
{
	Json::Value Root;
	std::vector<FENaiveSceneGraphNode*> AllNodes = GetAllNodes();
	// Remove root node. It is not needed in serialization
	AllNodes.erase(AllNodes.begin());

	for (size_t i = 0; i < AllNodes.size(); i++)
	{
		if (Filter != nullptr && !Filter(AllNodes[i]->GetEntity()))
			continue;

		Root["Nodes"][AllNodes[i]->GetObjectID()] = AllNodes[i]->ToJson(Filter);
	}

	return Root;
}

void FENaiveSceneGraph::FromJson(Json::Value Root)
{
	Clear();

	Json::Value Nodes = Root["Nodes"];
	std::unordered_map<std::string, FENaiveSceneGraphNode*> LoadedNodes;

	// First pass: Create all nodes
	for (Json::Value::const_iterator NodeIterator = Nodes.begin(); NodeIterator != Nodes.end(); NodeIterator++)
	{
		std::string NodeID = NodeIterator.key().asString();
		Json::Value NodeData = *NodeIterator;

		FENaiveSceneGraphNode* NewNode = new FENaiveSceneGraphNode(NodeData["Name"].asString());


		FEEntity* NewEntity = nullptr;
		FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(NodeData["Entity"]["FEObjectData"]);
		// Before passing data to node, we need to create entity
		NewEntity = ParentScene->CreateEntityOrphan(LoadedObjectData.Name, LoadedObjectData.ID);

		NewNode->Entity = NewEntity;
		NewNode->FromJson(NodeData);
		LoadedNodes[NodeID] = NewNode;
	}

	// Second pass: Set up parent-child relationships
	for (Json::Value::const_iterator NodeIterator = Nodes.begin(); NodeIterator != Nodes.end(); NodeIterator++)
	{
		std::string NodeID = NodeIterator.key().asString();
		Json::Value NodeData = *NodeIterator;

		FENaiveSceneGraphNode* CurrentNode = LoadedNodes[NodeID];
		std::string ParentID = NodeData["ParentID"].asString();

		FENaiveSceneGraphNode* ParentNode = LoadedNodes[ParentID];
		// If we can not find parent, it is root
		if (ParentNode == nullptr)
		{
			this->Root->AddChild(CurrentNode, false);
		}
		else
		{
			ParentNode->AddChild(CurrentNode, false); 
		}
	}
}

FEAABB FENaiveSceneGraph::GetNodeAABB(FEAABB& CumulativeAABB, FENaiveSceneGraphNode* TargetNode, std::function<bool(FEEntity*)> Filter)
{
	if (TargetNode == nullptr)
		return CumulativeAABB;

	if (TargetNode != Root)
	{
		if (Filter != nullptr && !Filter(TargetNode->GetEntity()))
			return CumulativeAABB;

		FEEntity* Entity = TargetNode->GetEntity();
		FEScene* Scene = Entity->GetParentScene();
		FEAABB NodeAABB = Scene->GetEntityAABB(Entity);
		CumulativeAABB = CumulativeAABB.Merge(NodeAABB);
	}
		
	for (size_t i = 0; i < TargetNode->Children.size(); i++)
		CumulativeAABB = CumulativeAABB.Merge(GetNodeAABB(CumulativeAABB, TargetNode->GetChildren()[i], Filter));

	return CumulativeAABB;
}