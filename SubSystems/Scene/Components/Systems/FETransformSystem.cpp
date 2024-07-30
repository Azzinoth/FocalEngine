#include "FETransformSystem.h"
using namespace FocalEngine;

FETransformSystem* FETransformSystem::Instance = nullptr;
FETransformSystem::FETransformSystem()
{
	RegisterOnComponentCallbacks();
}

void FETransformSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FETransformComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FETransformComponent>(OnMyComponentDestroy);
}

void FETransformSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FETransformComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	TransformComponent.ParentEntity = Entity;
}

void FETransformSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FETransformComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
}

FETransformSystem::~FETransformSystem() {};

void FETransformSystem::Update()
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (FEScene* Scene : ActiveScenes)
	{
		UpdateInternal(Scene->SceneGraph.GetRoot());
	}
}

void FETransformSystem::UpdateInternal(FENaiveSceneGraphNode* SubTreeRoot)
{
	if (SubTreeRoot->GetEntity() == nullptr)
		assert(false);

	FEScene* CurrentScene = SubTreeRoot->GetEntity()->GetParentScene();

	// If this is root node or parent is root node, then world space matrix is equal to local space matrix.
	FETransformComponent& CurrentTransform = SubTreeRoot->GetEntity()->GetComponent<FETransformComponent>();
	if (SubTreeRoot->GetParent() == nullptr || SubTreeRoot->GetParent() == CurrentScene->SceneGraph.GetRoot())
	{
		CurrentTransform.Update();
		CurrentTransform.WorldSpaceMatrix = CurrentTransform.LocalSpaceMatrix;
	}

	// FIX ME! instanced position should be updated if parent(or terrain) is updated.
	// right now it is not updated.

	//bool bWasDirty = CurrentTransform.IsDirty();
	//CurrentTransform.SetDirtyFlag(false);
	auto Children = SubTreeRoot->GetChildren();
	for (size_t i = 0; i < Children.size(); i++)
	{
		FETransformComponent& ChildTransform = Children[i]->GetEntity()->GetComponent<FETransformComponent>();

		ChildTransform.Update();
		ChildTransform.WorldSpaceMatrix = CurrentTransform.WorldSpaceMatrix * ChildTransform.LocalSpaceMatrix;
		//if (CurrentTransform.IsDirty())
		//	ChildTransform.SetDirtyFlag(true);
		UpdateInternal(Children[i]);
	}
}