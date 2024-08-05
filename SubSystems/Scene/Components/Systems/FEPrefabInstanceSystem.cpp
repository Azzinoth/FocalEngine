#include "FEPrefabInstanceSystem.h"
using namespace FocalEngine;

FEPrefabInstanceSystem* FEPrefabInstanceSystem::Instance = nullptr;
FEPrefabInstanceSystem::FEPrefabInstanceSystem()
{
	RegisterOnComponentCallbacks();
}

void FEPrefabInstanceSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FEPrefabInstanceComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FEPrefabInstanceComponent>(OnMyComponentDestroy);
}

void FEPrefabInstanceSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
}

void FEPrefabInstanceSystem::DuplicatePrefabInstanceComponent(FEEntity* EntityToWorkWith, FEEntity* NewEntity)
{
	if (EntityToWorkWith == nullptr || NewEntity == nullptr || !EntityToWorkWith->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEPrefabInstanceComponent& PrefabInstanceComponent = EntityToWorkWith->GetComponent<FEPrefabInstanceComponent>();

}

void FEPrefabInstanceSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
}

FEPrefabInstanceSystem::~FEPrefabInstanceSystem() {};

bool FEPrefabInstanceSystem::IsPrefabInstanceUnmodified(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEPrefabInstanceComponent>())
		return false;

	FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
	FEPrefab* Prefab = PrefabInstanceComponent.GetPrefab();
	if (Prefab == nullptr)
		return false;

	FEScene* EntityScene = Entity->GetParentScene();
	FENaiveSceneGraphNode* EntitySceneGraphNode = EntityScene->SceneGraph.GetNodeByEntityID(Entity->GetObjectID());

	// Because root node in prefab scene is always empty, and in working scene it's not, we need to skip it
	std::vector<FENaiveSceneGraphNode*> ChildNodesToCheck = EntitySceneGraphNode->GetChildren();
	std::vector<FENaiveSceneGraphNode*> PrefabChildNodesToCheck = Prefab->Scene->SceneGraph.GetRoot()->GetChildren();
	if (ChildNodesToCheck.size() != PrefabChildNodesToCheck.size())
		return false;

	for (size_t i = 0; i < PrefabChildNodesToCheck.size(); i++)
	{
		bool bHieraraichalEquivalence = SCENE_MANAGER.AreSceneGraphHierarchiesEquivalent(PrefabChildNodesToCheck[i], ChildNodesToCheck[i], true);
		if (!bHieraraichalEquivalence)
			return false;

	}

	// TODO: Check each node for equivalent components



	return true;
}

bool FEPrefabInstanceSystem::IsPartOfPrefabInstance(FEEntity* Entity)
{
	return GetParentPrefabInstanceIfAny(Entity) != nullptr;
}

FEEntity* FEPrefabInstanceSystem::GetParentPrefabInstanceIfAny(FEEntity* Entity)
{
	if (Entity == nullptr)
		return nullptr;

	if (Entity->HasComponent<FEPrefabInstanceComponent>())
		return Entity;

	FEScene* EntityScene = Entity->GetParentScene();
	FENaiveSceneGraphNode* EntitySceneGraphNode = EntityScene->SceneGraph.GetNodeByEntityID(Entity->GetObjectID());

	FENaiveSceneGraphNode* ResultNode = EntityScene->SceneGraph.GetFirstParentNodeWithComponent<FEPrefabInstanceComponent>(EntitySceneGraphNode);
	if (ResultNode != nullptr)
		return ResultNode->GetEntity();

	return nullptr;
}