#include "FENaiveSceneGraph.h"
using namespace FocalEngine;

FENaiveSceneGraph::FENaiveSceneGraph()
{
	Root = new FENaiveSceneGraphNode();
}

void FENaiveSceneGraph::Clear()
{
	for (size_t i = 0; i < Root->Children.size(); i++)
		RemoveNode(Root->Children[i]);
	
	delete Root;
	Root = new FENaiveSceneGraphNode();
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

std::string FENaiveSceneGraph::AddNode(FEObject* OldStyleEntity)
{
	FENaiveSceneGraphNode* NewEntity = new FENaiveSceneGraphNode(OldStyleEntity->GetName());
	NewEntity->OldStyleEntity = OldStyleEntity;
	Root->AddChild(NewEntity);
	return NewEntity->GetObjectID();
}

bool FENaiveSceneGraph::MoveNode(std::string NodeID, std::string NewParentID)
{
	FENaiveSceneGraphNode* NodeToMove = GetNode(NodeID);
	FENaiveSceneGraphNode* NewParent = GetNode(NewParentID);

	if (NodeToMove == nullptr || NewParent == nullptr)
		return false;

	if (IsDescendant(NodeToMove, NewParent))
		return false;

	FENaiveSceneGraphNode* OldParent = NodeToMove->GetParent();

	// Temporarily move the node
	RemoveNode(NodeToMove);
	AddNode(NewParent, NodeToMove);

	// Check for cycles
	if (HasCycle(GetRoot()))
	{
		// If a cycle is created, revert the change
		RemoveNode(NodeToMove);
		AddNode(OldParent, NodeToMove);
		return false;
	}

	return true;
}

void FENaiveSceneGraph::AddNode(FENaiveSceneGraphNode* NodeToAdd)
{
	Root->AddChild(NodeToAdd);
}

void FENaiveSceneGraph::AddNode(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd)
{
	Parent->AddChild(NodeToAdd);
}

void FENaiveSceneGraph::RemoveNode(FENaiveSceneGraphNode* NodeToRemove)
{
	if (NodeToRemove == Root)
		return;
	Root->RemoveChild(NodeToRemove);
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