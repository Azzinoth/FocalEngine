#include "FEVolumeSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetVolumeSystem()
{
	return FEVolumeSystem::GetInstancePointer();
}
#endif

FEVolumeSystem::FEVolumeSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FEVolumeComponent>(VolumeComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FEVolumeComponent>(VolumeComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FEVolumeComponent>(DuplicateVolumeComponent);

	/*InstancedLineShader = RESOURCE_MANAGER.CreateShader("instancedLine", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_VS.glsl").c_str()).c_str(),
																		 RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_FS.glsl").c_str()).c_str(),
																		 nullptr, nullptr, nullptr, nullptr,
																		 "7E0826291010377D564F6115");
	RESOURCE_MANAGER.SetTagInternal(InstancedLineShader, ENGINE_RESOURCE_TAG);*/
}

void FEVolumeSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FEVolumeComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FEVolumeComponent>(OnMyComponentDestroy);
}

void FEVolumeSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (VOLUME_SYSTEM.bInternalAdd)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return;
}

void FEVolumeSystem::DuplicateVolumeComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr || !SourceEntity->HasComponent<FEVolumeComponent>())
		return;

	FEVolumeComponent& VolumeComponent = SourceEntity->GetComponent<FEVolumeComponent>();

	VOLUME_SYSTEM.bInternalAdd = true;
	TargetEntity->AddComponent<FEVolumeComponent>();
	VOLUME_SYSTEM.bInternalAdd = false;
	FEVolumeComponent& NewVolumeComponent = TargetEntity->GetComponent<FEVolumeComponent>();
	//NewVolumeComponent.SetLineCollection(VolumeComponent.GetLineCollection());

	NewVolumeComponent = VolumeComponent;
}

void FEVolumeSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return;

	//FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
}

FEVolumeSystem::~FEVolumeSystem() {};

Json::Value FEVolumeSystem::VolumeComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
	{
		LOG.Add("FEVolumeSystem::VolumeComponentToJson Entity is nullptr or does not have FEVolumeComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();

	/*if (LineComponent.GetLineCollection() == nullptr)
	{
		Root["Line Collection ID"] = "none";
		return Root;
	}

	Root["Line Collection ID"] = LineComponent.GetLineCollection()->GetObjectID();*/
	
	return Root;
}

void FEVolumeSystem::VolumeComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FEVolumeSystem::VolumeComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	if (!Entity->HasComponent<FEVolumeComponent>())
		Entity->AddComponent<FEVolumeComponent>();

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();

	//if (!Root.isMember("Line Collection ID"))
	//{
	//	LOG.Add("FEVolumeSystem::VolumeComponentFromJson Root does not have 'Line Collection ID' member", "FE_LOG_ECS", FE_LOG_WARNING);
	//	return;
	//}

	//std::string LineCollectionID = Root["Line Collection ID"].asCString();
	//FELineCollection* LineCollection = RESOURCE_MANAGER.GetLineCollection(LineCollectionID);

	//if (LineCollection != nullptr)
	//	LineComponent.SetLineCollection(LineCollection);
}

void FEVolumeSystem::Render(FEEntity* Entity, FEEntity* Camera)
{
	if (Entity == nullptr || Camera == nullptr)
		return;

	if (!Entity->HasComponent<FEVolumeComponent>())
		return;

	if (!Entity->IsComponentVisible(ComponentVisibilityType::LINES))
		return;

	RenderVolumeComponent(Entity->GetComponent<FETransformComponent>(), Entity->GetComponent<FEVolumeComponent>(), Camera);
}

#include "FECameraSystem.h"
bool FEVolumeSystem::RenderVolumeComponent(FETransformComponent& TransformComponent, FEVolumeComponent& VolumeComponent, FEEntity* Camera)
{
	if (Camera == nullptr || !Camera->HasComponent<FECameraComponent>())
	{
		LOG.Add("FEVolumeSystem::RenderVolumeComponent: Camera is nullptr or does not have FECameraComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
	const FEViewport* CurrentCameraViewport = CurrentCameraComponent.GetViewport();
	if (CurrentCameraViewport == nullptr)
	{
		LOG.Add("FEVolumeSystem::RenderVolumeComponent: CurrentCameraViewport is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	return true;
}