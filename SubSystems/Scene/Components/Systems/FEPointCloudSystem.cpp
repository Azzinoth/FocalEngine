#include "FEPointCloudSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetPointCloudSystem()
{
	return FEPointCloudSystem::GetInstancePointer();
}
#endif

FEPointCloudSystem::FEPointCloudSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FEPointCloudComponent>(PointCloudComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FEPointCloudComponent>(PointCloudComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FEPointCloudComponent>(DuplicatePointCloudComponent);

	std::vector<FEShader*> StandardPointCloudShaderList = RESOURCE_MANAGER.GetShaderByName("StandardPointCloudShader");
	if (StandardPointCloudShaderList.empty())
	{
		LOG.Add("FEPointCloudSystem::FEPointCloudSystem: StandardPointCloudShader not found", "FE_LOG_RENDERING", FE_LOG_ERROR);
	}
	else
	{
		StandardPointCloudShader = StandardPointCloudShaderList[0];
	}
}

void FEPointCloudSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FEPointCloudComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FEPointCloudComponent>(OnMyComponentDestroy);
}

void FEPointCloudSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (POINT_CLOUD_SYSTEM.bInternalAdd)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEPointCloudComponent>())
		return;

	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
}

void FEPointCloudSystem::DuplicatePointCloudComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr || !SourceEntity->HasComponent<FEPointCloudComponent>())
		return;

	FEPointCloudComponent& PointCloudComponent = SourceEntity->GetComponent<FEPointCloudComponent>();

	POINT_CLOUD_SYSTEM.bInternalAdd = true;
	TargetEntity->AddComponent<FEPointCloudComponent>();
	POINT_CLOUD_SYSTEM.bInternalAdd = false;
	FEPointCloudComponent& NewPointCloudComponent = TargetEntity->GetComponent<FEPointCloudComponent>();

	NewPointCloudComponent = PointCloudComponent;
}

void FEPointCloudSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FEPointCloudComponent>())
		return;

	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
}

FEPointCloudSystem::~FEPointCloudSystem() {};

Json::Value FEPointCloudSystem::PointCloudComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FEPointCloudComponent>())
	{
		LOG.Add("FEPointCloudSystem::PointCloudComponentToJson Entity is nullptr or does not have FEPointCloudComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();

	if (PointCloudComponent.GetPointCloud() == nullptr)
	{
		Root["Point cloud ID"] = "none";
		return Root;
	}

	Root["Point cloud ID"] = PointCloudComponent.GetPointCloud()->GetObjectID();
	
	return Root;
}

void FEPointCloudSystem::PointCloudComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FEPointCloudSystem::PointCloudComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	if (!Entity->HasComponent<FEPointCloudComponent>())
		Entity->AddComponent<FEPointCloudComponent>();

	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();

	if (!Root.isMember("Point cloud ID"))
	{
		LOG.Add("FEPointCloudSystem::PointCloudComponentFromJson Root does not have Point cloud ID", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	std::string PointCloudID = Root["Point cloud ID"].asCString();
	FEPointCloud* PointCloud = RESOURCE_MANAGER.GetPointCloud(PointCloudID);

	if (PointCloud != nullptr)
		PointCloudComponent.SetPointCloud(PointCloud);
}

void FEPointCloudSystem::Render(FEEntity* Entity, FEEntity* Camera)
{
	if (Entity == nullptr || Camera == nullptr)
		return;

	if (!Entity->HasComponent<FEPointCloudComponent>())
		return;

	RenderPointCloudComponent(Entity->GetComponent<FETransformComponent>(), Entity->GetComponent<FEPointCloudComponent>(), Camera);
}

void FEPointCloudSystem::RenderPointCloudComponent(FETransformComponent& TransformComponent, FEPointCloudComponent& PointCloudComponent, FEEntity* Camera)
{
	FEPointCloud* PointCloud = PointCloudComponent.GetPointCloud();
	if (PointCloud == nullptr || Camera == nullptr)
		return;

	if (StandardPointCloudShader == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderPointCloudComponent: StandardPointCloudShader is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	glm::mat4 WorldMatrix = TransformComponent.GetWorldMatrix();
	glm::mat4 ViewMatrix = Camera->GetComponent<FECameraComponent>().GetViewMatrix();
	glm::mat4 ProjectionMatrix = Camera->GetComponent<FECameraComponent>().GetProjectionMatrix();

	StandardPointCloudShader->Start();
	StandardPointCloudShader->UpdateUniformData("FEWorldMatrix", WorldMatrix);
	StandardPointCloudShader->UpdateUniformData("FEViewMatrix", ViewMatrix);
	StandardPointCloudShader->UpdateUniformData("FEProjectionMatrix", ProjectionMatrix);
	if (PointCloudComponent.IsUsingGlobalColorOverride())
	{
		StandardPointCloudShader->UpdateUniformData("bUseGlobalColorOverride", true);
		StandardPointCloudShader->UpdateUniformData("FEGlobalColorOverride", PointCloudComponent.GetGlobalColorOverride());
	}
	else
	{
		StandardPointCloudShader->UpdateUniformData("bUseGlobalColorOverride", false);
		StandardPointCloudShader->UpdateUniformData("FEGlobalColorOverride", glm::vec3(0.0f));
	}
	StandardPointCloudShader->LoadUniformsDataToGPU();

	glBindVertexArray(PointCloud->GetVaoID());
	glDrawArrays(GL_POINTS, 0, static_cast<GLuint>(PointCloud->GetPointCount()));
}