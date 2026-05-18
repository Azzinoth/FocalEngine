#pragma once

template<typename T>
FENaiveSceneGraphNode* FENaiveSceneGraph::GetFirstRecursiveParentNodeWithComponent(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return nullptr;

	FENaiveSceneGraphNode* CurrentNode = Node;
	while (CurrentNode->GetParent() != nullptr)
	{
		CurrentNode = CurrentNode->GetParent();
		if (CurrentNode->GetEntity() == nullptr)
			return nullptr;

		if (CurrentNode->GetEntity()->HasComponent<T>())
			return CurrentNode;
	}

	return nullptr;
}

template<typename T>
FENaiveSceneGraphNode* FENaiveSceneGraph::GetFirstImmediateChildNodeWithComponent(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return nullptr;

	for (FENaiveSceneGraphNode* Child : Node->GetChildren())
	{
		if (Child->GetEntity() != nullptr && Child->GetEntity()->HasComponent<T>())
			return Child;
	}

	return nullptr;
}

template<typename T>
FENaiveSceneGraphNode* FENaiveSceneGraph::GetFirstRecursiveChildNodeWithComponent(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return nullptr;

	// Check immediate children first so the shallowest matching descendant wins.
	for (FENaiveSceneGraphNode* Child : Node->GetChildren())
	{
		if (Child->GetEntity() != nullptr && Child->GetEntity()->HasComponent<T>())
			return Child;
	}

	// Then recurse into each subtree.
	for (FENaiveSceneGraphNode* Child : Node->GetChildren())
	{
		FENaiveSceneGraphNode* Found = GetFirstRecursiveChildNodeWithComponent<T>(Child);
		if (Found != nullptr)
			return Found;
	}

	return nullptr;
}