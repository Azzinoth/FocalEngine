#include "FENaiveSceneGraph.h"
using namespace FocalEngine;

FENaiveSceneGraph::FENaiveSceneGraph()
{
	Root = new FENaiveSceneGraphNode();
}

void FENaiveSceneGraph::Clear()
{
	bClearing = true;
	DeleteNode(Root);
	Root = new FENaiveSceneGraphNode();
	bClearing = false;
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

FENaiveSceneGraphNode* FENaiveSceneGraph::GetNodeByOldEntityID(std::string OldEntityID)
{
	// Root can'n have OldStyleEntity
	return Root->GetChildByOldEntityID(OldEntityID);
}

std::string FENaiveSceneGraph::AddNode(FEObject* OldStyleEntity, bool bPreserveWorldTransform)
{
	FENaiveSceneGraphNode* NewEntity = new FENaiveSceneGraphNode(OldStyleEntity->GetName());
	NewEntity->OldStyleEntity = OldStyleEntity;
	Root->AddChild(NewEntity, bPreserveWorldTransform);

	return NewEntity->GetObjectID();
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
	AddNode(NewParent, NodeToMove, bPreserveWorldTransform);

	// Check for cycles
	if (HasCycle(GetRoot()))
	{
		// If a cycle is created, revert the change
		DetachNode(NodeToMove, bPreserveWorldTransform);
		AddNode(OldParent, NodeToMove, bPreserveWorldTransform);
		return false;
	}

	return true;
}

void FENaiveSceneGraph::AddNode(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform)
{
	Root->AddChild(NodeToAdd, bPreserveWorldTransform);
}

void FENaiveSceneGraph::AddNode(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform)
{
	Parent->AddChild(NodeToAdd, bPreserveWorldTransform);
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

Json::Value FENaiveSceneGraph::ToJson()
{
	Json::Value Root;
	std::vector<FENaiveSceneGraphNode*> AllNodes = GetAllNodes();
	// Remove root node
	AllNodes.erase(AllNodes.begin());

	for (size_t i = 0; i < AllNodes.size(); i++)
		Root["Nodes"][AllNodes[i]->GetObjectID()] = AllNodes[i]->ToJson();

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