#include "FETransformSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetTransformSystem()
{
	return FETransformSystem::GetInstancePointer();
}
#endif

FETransformSystem::FETransformSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FETransformComponent>(TransfromComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FETransformComponent>(TransfromComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FETransformComponent>(DuplicateTransformComponent);
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
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active);
	for (FEScene* Scene : ActiveScenes)
	{
		UpdateInternal(Scene->SceneGraph.GetRoot());
	}
}

void FETransformSystem::DuplicateTransformComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr)
		return;

	TargetEntity->GetComponent<FETransformComponent>() = SourceEntity->GetComponent<FETransformComponent>();
	TargetEntity->GetComponent<FETransformComponent>().ParentEntity = TargetEntity;
}

void FETransformSystem::UpdateInternal(FENaiveSceneGraphNode* SubTreeRoot)
{
	// If it is root node, then work only on children
	if (SubTreeRoot->GetEntity() == nullptr)
	{
		auto Children = SubTreeRoot->GetChildren();
		for (size_t i = 0; i < Children.size(); i++)
		{
			UpdateInternal(Children[i]);
		}

		return;
	}

	FEScene* CurrentScene = SubTreeRoot->GetEntity()->GetParentScene();

	// If this is root node or parent is root node, then world space matrix is equal to local space matrix.
	FETransformComponent& CurrentTransform = SubTreeRoot->GetEntity()->GetComponent<FETransformComponent>();
	if (SubTreeRoot->GetParent() == nullptr || SubTreeRoot->GetParent() == CurrentScene->SceneGraph.GetRoot())
	{
		CurrentTransform.Update();
		CurrentTransform.WorldSpaceMatrix = CurrentTransform.LocalSpaceMatrix;
	}

	// FIXME: Instanced position should update when parent (or terrain) is updated, currently no update occurs.
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

Json::Value FETransformSystem::TransfromComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();

	glm::vec3 Position = TransformComponent.GetPosition();
	Root["Position"]["X"] = Position.x;
	Root["Position"]["Y"] = Position.y;
	Root["Position"]["Z"] = Position.z;

	glm::quat Rotation = TransformComponent.GetQuaternion();
	Root["Rotation"]["X"] = Rotation.x;
	Root["Rotation"]["Y"] = Rotation.y;
	Root["Rotation"]["Z"] = Rotation.z;
	Root["Rotation"]["W"] = Rotation.w;

	Root["Scale"]["UniformScaling"] = TransformComponent.IsUniformScalingSet();
	glm::vec3 Scale = TransformComponent.GetScale();
	Root["Scale"]["X"] = Scale.x;
	Root["Scale"]["Y"] = Scale.y;
	Root["Scale"]["Z"] = Scale.z;

	return Root;
}

void FETransformSystem::TransfromComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();

	glm::vec3 Position;
	Position.x = Root["Position"]["X"].asFloat();
	Position.y = Root["Position"]["Y"].asFloat();
	Position.z = Root["Position"]["Z"].asFloat();
	TransformComponent.SetPosition(Position);

	glm::quat Rotation;
	Rotation.x = Root["Rotation"]["X"].asFloat();
	Rotation.y = Root["Rotation"]["Y"].asFloat();
	Rotation.z = Root["Rotation"]["Z"].asFloat();
	Rotation.w = Root["Rotation"]["W"].asFloat();
	TransformComponent.SetQuaternion(Rotation);

	bool bUniformScaling = Root["Scale"]["UniformScaling"].asBool();
	TransformComponent.SetUniformScaling(bUniformScaling);

	glm::vec3 Scale;
	Scale.x = Root["Scale"]["X"].asFloat();
	Scale.y = Root["Scale"]["Y"].asFloat();
	Scale.z = Root["Scale"]["Z"].asFloat();
	TransformComponent.SetScale(Scale);
}