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

	const std::string EngineFolder = RESOURCE_MANAGER.GetEngineFolder();

	VolumetricShaders.push_back(RESOURCE_MANAGER.CreateShader("FEVolumetricShader_Basic", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//Volumetric//FE_Volumetric_VS.glsl").c_str()).c_str(),
																						  RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//Volumetric//FE_Volumetric_FS_Basic.glsl").c_str()).c_str(),
																						  nullptr, nullptr, nullptr, nullptr, "43590632272B4B5E403C096C"));

	VolumetricShaders.push_back(RESOURCE_MANAGER.CreateShader("FEVolumetricShader_Cleaned", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//Volumetric//FE_Volumetric_VS.glsl").c_str()).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//Volumetric//FE_Volumetric_FS_Cleaned.glsl").c_str()).c_str(),
																							nullptr, nullptr, nullptr, nullptr, "391E240A020F67670C16001E"));
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

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
	if (VolumeComponent.VolumetricShader == nullptr && !VOLUME_SYSTEM.VolumetricShaders.empty())
		VolumeComponent.VolumetricShader = VOLUME_SYSTEM.VolumetricShaders[0];
}

std::vector<FEShader*> FEVolumeSystem::GetVolumetricShaders()
{
	return VolumetricShaders;
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
	NewVolumeComponent.VolumetricShader = VolumeComponent.VolumetricShader;
	NewVolumeComponent.VolumetricTexture = VolumeComponent.VolumetricTexture;

	NewVolumeComponent = VolumeComponent;
}

void FEVolumeSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return;
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

	Root["VolumetricShader"] = VolumeComponent.VolumetricShader == nullptr ? "none" : VolumeComponent.VolumetricShader->GetObjectID();
	Root["3DTexture"] = VolumeComponent.VolumetricTexture == nullptr ? "none" : VolumeComponent.VolumetricTexture->GetObjectID();
	
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

	if (Root.isMember("VolumetricShader") && Root["VolumetricShader"].isString())
	{
		std::string ShaderID = Root["VolumetricShader"].asString();
		if (ShaderID != "none")
			VolumeComponent.VolumetricShader = RESOURCE_MANAGER.GetShader(ShaderID);
	}

	if (Root.isMember("3DTexture") && Root["3DTexture"].isString())
	{
		std::string TextureID = Root["3DTexture"].asString();
		if (TextureID != "none")
			VolumeComponent.SetVolumetricTexture(RESOURCE_MANAGER.GetTexture(TextureID));
	}
}

void FEVolumeSystem::Render(FEEntity* Entity, FEEntity* Camera)
{
	if (Entity == nullptr || Camera == nullptr)
		return;

	if (!Entity->HasComponent<FEVolumeComponent>())
		return;

	if (!Entity->IsComponentVisible(ComponentVisibilityType::VOLUME))
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

	FEVolumeComponent& CurrentVolumeComponent = VolumeComponent;
	if (CurrentVolumeComponent.VolumetricShader == nullptr)
	{
		LOG.Add("FEVolumeSystem::RenderVolumeComponent: CurrentVolumeComponent.VolumetricShader is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (CurrentVolumeComponent.VolumetricTexture == nullptr)
	{
		LOG.Add("FEVolumeSystem::RenderVolumeComponent: CurrentVolumeComponent.VolumetricTexture is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	CurrentVolumeComponent.VolumetricShader->UpdateUniformData("NearPlane", CurrentCameraComponent.GetNearPlane());
	CurrentVolumeComponent.VolumetricShader->UpdateUniformData("FarPlane", CurrentCameraComponent.GetFarPlane());

	CurrentVolumeComponent.VolumetricShader->UpdateUniformData("invViewMatrix", glm::inverse(CurrentCameraComponent.GetViewMatrix()));
	CurrentVolumeComponent.VolumetricShader->UpdateUniformData("invProjectionMatrix", glm::inverse(CurrentCameraComponent.GetProjectionMatrix()));

	CurrentVolumeComponent.VolumetricShader->LoadUniformsDataToGPU();

	CurrentVolumeComponent.VolumetricTexture->Bind(2);

	FEMesh* ScreenQuad = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	FE_GL_ERROR(glBindVertexArray(ScreenQuad->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, ScreenQuad->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	return true;
}