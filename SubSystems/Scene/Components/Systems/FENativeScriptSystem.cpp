#include "FENativeScriptSystem.h"
using namespace FocalEngine;

FENativeScriptSystem* FENativeScriptSystem::Instance = nullptr;
FENativeScriptSystem::FENativeScriptSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FENativeScriptComponent>(NativeScriptComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FENativeScriptComponent>(NativeScriptComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FENativeScriptComponent>(DuplicateNativeScriptComponent);
}

void FENativeScriptSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FENativeScriptComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FENativeScriptComponent>(OnMyComponentDestroy);
}

void FENativeScriptSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
	//NativeScriptComponent.ParentEntity = Entity;
	//auto& nsc = entity->GetComponent<FENativeScriptComponent>();
	/*if (NativeScriptComponent.CreateScript)
		NativeScriptComponent.CreateScript(NativeScriptComponent);

	if (NativeScriptComponent.Instance)
	{
		NativeScriptComponent.Instance->Entity = Entity;
		NativeScriptComponent.Instance->OnCreate();
	}*/
}

void FENativeScriptSystem::Init(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
	//NativeScriptComponent.ParentEntity = Entity;

	if (NativeScriptComponent.CreateScript)
		NativeScriptComponent.CreateScript(NativeScriptComponent);

	if (NativeScriptComponent.CoreInstance)
	{
		NativeScriptComponent.CoreInstance->ParentEntity = Entity;
		NativeScriptComponent.CoreInstance->OnCreate();
	}
}

void FENativeScriptSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FENativeScriptComponent>())
		return;

	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();
}

FENativeScriptSystem::~FENativeScriptSystem() {};

void FENativeScriptSystem::Update()
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (FEScene* Scene : ActiveScenes)
	{
		// TO_DO: change GetEntityListWith to GetEntityListWithComponent.
		std::vector<FEEntity*> Entities = Scene->GetEntityListWith<FENativeScriptComponent>();

		for (size_t i = 0; i < Entities.size(); i++)
		{
			FENativeScriptComponent& NativeScriptComponent = Entities[i]->GetComponent<FENativeScriptComponent>();
			if (NativeScriptComponent.CoreInstance)
				NativeScriptComponent.CoreInstance->OnUpdate(0.0f);
			//UpdateInternal(Scene->SceneGraph.GetRoot());
		}
		
	}
}

void FENativeScriptSystem::DuplicateNativeScriptComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr)
		return;

	TargetEntity->GetComponent<FENativeScriptComponent>() = SourceEntity->GetComponent<FENativeScriptComponent>();
	//TargetEntity->GetComponent<FENativeScriptComponent>().ParentEntity = TargetEntity;
}

Json::Value FENativeScriptSystem::NativeScriptComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();

	/*glm::vec3 Position = TransformComponent.GetPosition();
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
	Root["Scale"]["Z"] = Scale.z;*/

	return Root;
}

void FENativeScriptSystem::NativeScriptComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	FENativeScriptComponent& NativeScriptComponent = Entity->GetComponent<FENativeScriptComponent>();

	/*glm::vec3 Position;
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
	TransformComponent.SetScale(Scale);*/
}