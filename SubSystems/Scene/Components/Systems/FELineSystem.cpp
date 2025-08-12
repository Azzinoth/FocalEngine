#include "FELineSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetLineSystem()
{
	return FELineSystem::GetInstancePointer();
}
#endif

FELineSystem::FELineSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FELineComponent>(LineComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FELineComponent>(LineComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FELineComponent>(DuplicateLineComponent);

	InstancedLineShader = RESOURCE_MANAGER.CreateShader("instancedLine", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_VS.glsl").c_str()).c_str(),
																		 RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_FS.glsl").c_str()).c_str(),
																		 nullptr, nullptr, nullptr, nullptr,
																		 "7E0826291010377D564F6115");
	RESOURCE_MANAGER.SetTagInternal(InstancedLineShader, ENGINE_RESOURCE_TAG);
}

void FELineSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FELineComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FELineComponent>(OnMyComponentDestroy);
}

void FELineSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (LINE_SYSTEM.bInternalAdd)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FELineComponent>())
		return;
}

void FELineSystem::DuplicateLineComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr || !SourceEntity->HasComponent<FELineComponent>())
		return;

	FELineComponent& LineComponent = SourceEntity->GetComponent<FELineComponent>();

	LINE_SYSTEM.bInternalAdd = true;
	TargetEntity->AddComponent<FELineComponent>();
	LINE_SYSTEM.bInternalAdd = false;
	FELineComponent& NewLineComponent = TargetEntity->GetComponent<FELineComponent>();
	NewLineComponent.SetLineCollection(LineComponent.GetLineCollection());

	NewLineComponent = LineComponent;
}

void FELineSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FELineComponent>())
		return;

	FELineComponent& LineComponent = Entity->GetComponent<FELineComponent>();
}

FELineSystem::~FELineSystem() {};

Json::Value FELineSystem::LineComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FELineComponent>())
	{
		LOG.Add("FELineSystem::LineComponentToJson Entity is nullptr or does not have FELineComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FELineComponent& LineComponent = Entity->GetComponent<FELineComponent>();

	if (LineComponent.GetLineCollection() == nullptr)
	{
		Root["Line Collection ID"] = "none";
		return Root;
	}

	Root["Line Collection ID"] = LineComponent.GetLineCollection()->GetObjectID();
	
	return Root;
}

void FELineSystem::LineComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FELineSystem::LineComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	if (!Entity->HasComponent<FELineComponent>())
		Entity->AddComponent<FELineComponent>();

	FELineComponent& LineComponent = Entity->GetComponent<FELineComponent>();
	if (!Root.isMember("Line Collection ID"))
	{
		LOG.Add("FELineSystem::LineComponentFromJson Root does not have 'Line Collection ID' member", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	std::string LineCollectionID = Root["Line Collection ID"].asCString();
	FELineCollection* LineCollection = RESOURCE_MANAGER.GetLineCollection(LineCollectionID);

	if (LineCollection != nullptr)
		LineComponent.SetLineCollection(LineCollection);
}

void FELineSystem::Render(FEEntity* Entity, FEEntity* Camera)
{
	if (Entity == nullptr || Camera == nullptr)
		return;

	if (!Entity->HasComponent<FELineComponent>())
		return;

	if (!Entity->IsComponentVisible(ComponentVisibilityType::LINES))
		return;

	RenderLineComponent(Entity->GetComponent<FETransformComponent>(), Entity->GetComponent<FELineComponent>(), Camera);
}

#include "FECameraSystem.h"
bool FELineSystem::RenderLineComponent(FETransformComponent& TransformComponent, FELineComponent& LineComponent, FEEntity* Camera)
{
	if (Camera == nullptr || !Camera->HasComponent<FECameraComponent>())
	{
		LOG.Add("FELineSystem::RenderLineComponent: Camera is nullptr or does not have FECameraComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
	const FEViewport* CurrentCameraViewport = CurrentCameraComponent.GetViewport();
	if (CurrentCameraViewport == nullptr)
	{
		LOG.Add("FELineSystem::RenderLineComponent: CurrentCameraViewport is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FELineCollection* LineCollection = LineComponent.GetLineCollection();
	if (LineCollection == nullptr || Camera == nullptr)
	{
		LOG.Add("FELineSystem::RenderLineComponent: LineCollection is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	InstancedLineShader->Start();
	InstancedLineShader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	InstancedLineShader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	InstancedLineShader->UpdateUniformData("FEWorldMatrix", TransformComponent.GetWorldMatrix());
	InstancedLineShader->UpdateUniformData("resolution", glm::vec2(CurrentCameraViewport->GetWidth(), CurrentCameraViewport ->GetHeight()));
	InstancedLineShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(LineCollection->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glEnableVertexAttribArray(1));
	FE_GL_ERROR(glEnableVertexAttribArray(2));
	FE_GL_ERROR(glEnableVertexAttribArray(3));
	FE_GL_ERROR(glEnableVertexAttribArray(4));
	FE_GL_ERROR(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, LineCollection->GetLineCount()));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glDisableVertexAttribArray(1));
	FE_GL_ERROR(glDisableVertexAttribArray(2));
	FE_GL_ERROR(glDisableVertexAttribArray(3));
	FE_GL_ERROR(glDisableVertexAttribArray(4));
	FE_GL_ERROR(glBindVertexArray(0));

	InstancedLineShader->Stop();

	return true;
}