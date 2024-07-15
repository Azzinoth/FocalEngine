#include "FESkyDomeSystem.h"
using namespace FocalEngine;

FESkyDomeSystem* FESkyDomeSystem::Instance = nullptr;
FEGameModel* FESkyDomeSystem::SkyDomeGameModel = nullptr;
FEGameModelComponent* FESkyDomeSystem::SkyDomeGameModelComponent = nullptr;

FESkyDomeSystem::FESkyDomeSystem()
{
	FEMaterial* SkyDomeMaterial = RESOURCE_MANAGER.CreateMaterial("skyDomeMaterial");
	RESOURCE_MANAGER.Materials.erase(SkyDomeMaterial->GetObjectID());
	SkyDomeMaterial->SetID("5A649B9E0F36073D4939313H");
	SkyDomeMaterial->Shader = RESOURCE_MANAGER.CreateShader("FESkyDome", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_VS.glsl").c_str()).c_str(),
																		 RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//SkyDome//FE_SkyDome_FS.glsl").c_str()).c_str());
	RESOURCE_MANAGER.Shaders.erase(SkyDomeMaterial->Shader->GetObjectID());
	SkyDomeMaterial->Shader->SetID("3A69744E831A574E4857361B");
	RESOURCE_MANAGER.Shaders[SkyDomeMaterial->Shader->GetObjectID()] = SkyDomeMaterial->Shader;

	RESOURCE_MANAGER.MakeShaderStandard(SkyDomeMaterial->Shader);
	RESOURCE_MANAGER.MakeMaterialStandard(SkyDomeMaterial);

	SkyDomeGameModel = new FEGameModel(RESOURCE_MANAGER.GetMesh("7F251E3E0D08013E3579315F"), RESOURCE_MANAGER.GetMaterial("5A649B9E0F36073D4939313H"/*"skyDomeMaterial"*/), "skyDomeGameModel");
	SkyDomeGameModel->SetID("17271E603508013IO77931TY");
	RESOURCE_MANAGER.MakeGameModelStandard(SkyDomeGameModel);

	SkyDomeGameModelComponent = new FEGameModelComponent(SkyDomeGameModel);

	RegisterOnComponentCallbacks();
	SCENE.AddOnSceneClearCallback(std::bind(&FESkyDomeSystem::OnSceneClear, this));
}

void FESkyDomeSystem::RegisterOnComponentCallbacks()
{
	SCENE.RegisterOnComponentConstructCallback<FESkyDomeComponent>(OnMyComponentAdded);
	SCENE.RegisterOnComponentDestroyCallback<FESkyDomeComponent>(OnMyComponentDestroy);
}

void FESkyDomeSystem::OnSceneClear()
{
	RegisterOnComponentCallbacks();
}

void FESkyDomeSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FESkyDomeComponent>())
		return;
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
	return SkyDomeGameModelComponent->IsVisible();
}

void FESkyDomeSystem::SetEnabled(bool NewValue)
{
	SkyDomeGameModelComponent->SetVisibility(NewValue);
}

void FESkyDomeSystem::AddToEntity(FEEntity* EntityToAdd)
{
	EntityToAdd->AddComponent<FESkyDomeComponent>();
}