#include "FESkyDomeSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetSkyDomeSystem()
{
	return FESkyDomeSystem::GetInstancePointer();
}
#endif

FEGameModel* FESkyDomeSystem::SkyDomeGameModel = nullptr;

FESkyDomeSystem::FESkyDomeSystem()
{
	FEMaterial* SkyDomeMaterial = RESOURCE_MANAGER.CreateMaterial("skyDomeMaterial", "5A649B9E0F36073D4939313H");
	RESOURCE_MANAGER.SetTagIternal(SkyDomeMaterial, ENGINE_RESOURCE_TAG);
	SkyDomeMaterial->Shader = RESOURCE_MANAGER.CreateShader("FESkyDome", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_VS.glsl").c_str()).c_str(),
																		 RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_FS.glsl").c_str()).c_str(),
																		 nullptr, nullptr, nullptr, nullptr,
																		 "3A69744E831A574E4857361B");
	RESOURCE_MANAGER.SetTagIternal(SkyDomeMaterial->Shader, ENGINE_RESOURCE_TAG);
	
	SkyDomeGameModel = new FEGameModel(RESOURCE_MANAGER.GetMesh("7F251E3E0D08013E3579315F"), RESOURCE_MANAGER.GetMaterial("5A649B9E0F36073D4939313H"/*"skyDomeMaterial"*/), "skyDomeGameModel");
	SkyDomeGameModel->SetID("17271E603508013IO77931TY");
	RESOURCE_MANAGER.SetTagIternal(SkyDomeGameModel, ENGINE_RESOURCE_TAG);

	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FESkyDomeComponent>(SkyDomeComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FESkyDomeComponent>(SkyDomeComponentFromJson);
}

void FESkyDomeSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FESkyDomeComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FESkyDomeComponent>(OnMyComponentDestroy);
}

void FESkyDomeSystem::OnSceneClear()
{
	RegisterOnComponentCallbacks();
}

void FESkyDomeSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FESkyDomeComponent>())
		return;

	Entity->AddComponent<FEGameModelComponent>(SkyDomeGameModel);
	Entity->GetComponent<FEGameModelComponent>().SetVisibility(false);
}

void FESkyDomeSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (bIsSceneClearing)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
		return;
}

FESkyDomeSystem::~FESkyDomeSystem() {};

bool FESkyDomeSystem::IsEnabled()
{
	return bEnabled;
}

void FESkyDomeSystem::SetEnabled(bool NewValue)
{
	bEnabled = NewValue;
}

Json::Value FESkyDomeSystem::SkyDomeComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FESkyDomeComponent>())
	{
		LOG.Add("FESkyDomeSystem::SkyDomeComponentToJson Entity is nullptr or does not have FESkyDomeComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FESkyDomeComponent& SkyDomeComponent = Entity->GetComponent<FESkyDomeComponent>();

	Root["Enabled"] = SKY_DOME_SYSTEM.bEnabled;

	return Root;
}

void FESkyDomeSystem::SkyDomeComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FESkyDomeSystem::SkyDomeComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	// TODO: Make sky dome component not depend on game model component
	// GameModelComponent could load faster, so we need to remove it
	if (Entity->HasComponent<FEGameModelComponent>())
		Entity->RemoveComponent<FEGameModelComponent>();

	Entity->AddComponent<FESkyDomeComponent>();
	FESkyDomeComponent& SkyDomeComponent = Entity->GetComponent<FESkyDomeComponent>();
}