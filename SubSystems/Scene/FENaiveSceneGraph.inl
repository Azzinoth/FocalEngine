#pragma once

template<typename T>
FENaiveSceneGraphNode* FENaiveSceneGraph::GetFirstParentNodeWithComponent(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return nullptr;
	
	FENaiveSceneGraphNode* CurrentNode = Node;
	while (CurrentNode->GetParent() != nullptr)
	{
		CurrentNode = CurrentNode->GetParent();
		if (CurrentNode->GetEntity()->HasComponent<T>())
		{
			return CurrentNode;
		}
	}
	return nullptr;
}

template<typename T>
FENaiveSceneGraphNode* FENaiveSceneGraph::GetFirstChildNodeWithComponent(FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return nullptr;

	for (FENaiveSceneGraphNode* Child : Node->GetChildren())
	{
		if (Child->GetEntity()->HasComponent<T>())
		{
			return Child;
		}
	}

	return nullptr;
}