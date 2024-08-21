#include "FEVirtualUISystem.h"
#include "../../Renderer/FERenderer.h"
#include "../../../FEngine.h"
using namespace FocalEngine;

FEVirtualUISystem::FEVirtualUISystem()
{
	CanvasMaterial = RESOURCE_MANAGER.CreateMaterial();
	CanvasMaterial->SetName("VirtualUISystem_CanvasMaterial");
	RESOURCE_MANAGER.SetTagIternal(CanvasMaterial, ENGINE_RESOURCE_TAG);
	CanvasMaterial->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

	DummyGameModel = RESOURCE_MANAGER.CreateGameModel();
	DummyGameModel->SetName("VirtualUISystem_DummyGameModel");
	DummyGameModel->SetMaterial(CanvasMaterial);
	RESOURCE_MANAGER.SetTagIternal(DummyGameModel, ENGINE_RESOURCE_TAG);
	DummyGameModelComponent.SetGameModel(DummyGameModel);

	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FEVirtualUIComponent>(VirtualUIComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FEVirtualUIComponent>(VirtualUIComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FEVirtualUIComponent>(DuplicateVirtualUIComponent);
}

void FEVirtualUISystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FEVirtualUIComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FEVirtualUIComponent>(OnMyComponentDestroy);
}

void FEVirtualUISystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVirtualUIComponent>())
		return;

	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();
	VirtualUIComponent.ParentEntity = Entity;
	VirtualUIComponent.SetRenderFunction(DummyRenderFunction);
}

void FEVirtualUISystem::DuplicateVirtualUIComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr || !SourceEntity->HasComponent<FEVirtualUIComponent>())
		return;

	FEVirtualUIComponent& VirtualUIComponent = SourceEntity->GetComponent<FEVirtualUIComponent>();
	TargetEntity->AddComponent<FEVirtualUIComponent>();
	FEVirtualUIComponent& NewVirtualUIComponent = TargetEntity->GetComponent<FEVirtualUIComponent>();

	NewVirtualUIComponent.SetWindowToListen(VirtualUIComponent.GetWindowToListen());
	NewVirtualUIComponent.SetRenderFunction(VirtualUIComponent.GetRenderFunction());

	NewVirtualUIComponent.SetMouseButtonPassThrough(VirtualUIComponent.bMouseButtonPassThrough);
	NewVirtualUIComponent.SetMouseMovePassThrough(VirtualUIComponent.bMouseMovePassThrough);
	NewVirtualUIComponent.SetCharPassThrough(VirtualUIComponent.bCharPassThrough);
	NewVirtualUIComponent.SetKeyPassThrough(VirtualUIComponent.bKeyPassThrough);
	NewVirtualUIComponent.SetDropPassThrough(VirtualUIComponent.bDropPassThrough);
	NewVirtualUIComponent.SetScrollPassThrough(VirtualUIComponent.bScrollPassThrough);
	
	NewVirtualUIComponent.SetVisibility(VirtualUIComponent.IsVisible());
	NewVirtualUIComponent.SetInputActive(VirtualUIComponent.IsInputActive());

	NewVirtualUIComponent.InvokeResize(VirtualUIComponent.GetWidth(), VirtualUIComponent.GetHeight());
}

void FEVirtualUISystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVirtualUIComponent>())
		return;

	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();
}

FEVirtualUISystem::~FEVirtualUISystem() {};

Json::Value FEVirtualUISystem::VirtualUIComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FEVirtualUIComponent>())
	{
		LOG.Add("FEVirtualUISystem::VirtualUIComponentToJson Entity is nullptr or does not have FEVirtualUIComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();

	// TO-DO: Save window to listen to?
	//Root["Window to listen"] = VirtualUIComponent.GetWindowToListen() != nullptr;

	Root["Canvas mesh ID"] = VirtualUIComponent.CanvasMesh->GetObjectID();
	Root["Internal resolution"]["Width"] = static_cast<int>(VirtualUIComponent.GetCanvasResolution().x);
	Root["Internal resolution"]["Height"] = static_cast<int>(VirtualUIComponent.GetCanvasResolution().y);

	Root["Visibility"] = VirtualUIComponent.IsVisible();
	Root["Input Active"] = VirtualUIComponent.IsInputActive();

	// TO-DO: How to save function pointers?
	//Root["Render Function"] = VirtualUIComponent.GetRenderFunction() != nullptr;

	Root["Mouse Button PassThrough"] = VirtualUIComponent.IsMouseButtonPassThroughActive();
	Root["Mouse Move PassThrough"] = VirtualUIComponent.IsMouseMovePassThroughActive();
	Root["Scroll PassThrough"] = VirtualUIComponent.IsScrollPassThroughActive();
	Root["Char PassThrough"] = VirtualUIComponent.IsCharPassThroughActive();
	Root["Key PassThrough"] = VirtualUIComponent.IsKeyPassThroughActive();
	Root["Drop PassThrough"] = VirtualUIComponent.IsDropPassThroughActive();

	return Root;
}

void FEVirtualUISystem::VirtualUIComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FEVirtualUISystem::VirtualUIComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}
	glm::ivec2 Resolution = glm::vec2(Root["Internal resolution"]["Width"].asInt(), Root["Internal resolution"]["Height"].asInt());
	FEMesh* CanvasMesh = RESOURCE_MANAGER.GetMesh(Root["Canvas mesh ID"].asString());
	Entity->AddComponent<FEVirtualUIComponent>(Resolution.x, Resolution.y, CanvasMesh);
	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();

	//VirtualUIComponent.SetRenderFunction(TestRender);
	VirtualUIComponent.SetWindowToListen(APPLICATION.GetMainWindow());
	
	VirtualUIComponent.SetCanvasResolution(Resolution);

	VirtualUIComponent.SetMouseButtonPassThrough(Root["Mouse Button PassThrough"].asBool());
	VirtualUIComponent.SetMouseMovePassThrough(Root["Mouse Move PassThrough"].asBool());
	VirtualUIComponent.SetScrollPassThrough(Root["Scroll PassThrough"].asBool());
	VirtualUIComponent.SetCharPassThrough(Root["Char PassThrough"].asBool());
	VirtualUIComponent.SetKeyPassThrough(Root["Key PassThrough"].asBool());
	VirtualUIComponent.SetDropPassThrough(Root["Drop PassThrough"].asBool());
}

void FEVirtualUISystem::RenderVirtualUIComponent(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVirtualUIComponent>())
	{
		LOG.Add("FEVirtualUISystem::RenderVirtualUIComponent Entity is nullptr or does not have FEVirtualUIComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}
	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();

	RenderVirtualUIComponent(Entity, CanvasMaterial);
	CanvasMaterial->ClearAllTexturesInfo();
}

void FEVirtualUISystem::RenderVirtualUIComponent(FEEntity* Entity, FEMaterial* ForceMaterial)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVirtualUIComponent>())
	{
		LOG.Add("FEVirtualUISystem::RenderVirtualUIComponent Entity is nullptr or does not have FEVirtualUIComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	if (ForceMaterial == nullptr)
	{
		LOG.Add("FEVirtualUISystem::RenderVirtualUIComponent ForceMaterial is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}
	FEVirtualUIComponent& VirtualUIComponent = Entity->GetComponent<FEVirtualUIComponent>();

	ForceMaterial->SetAlbedoMap(VirtualUIComponent.Framebuffer->GetColorAttachment());
	DummyGameModel->SetMaterial(ForceMaterial);
	DummyGameModel->SetMesh(VirtualUIComponent.CanvasMesh);

	RENDERER.RenderGameModelComponent(DummyGameModelComponent, Entity->GetComponent<FETransformComponent>(), Entity->GetParentScene(), nullptr, true);

	ForceMaterial->SetAlbedoMap(nullptr);
	DummyGameModel->SetMaterial(nullptr);
	DummyGameModel->SetMesh(nullptr);
	CanvasMaterial->SetAlbedoMap(nullptr);
}

void FEVirtualUISystem::Update()
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (size_t i = 0; i < ActiveScenes.size(); i++)
	{
		FEEntity* CameraEntity = CAMERA_SYSTEM.GetMainCameraEntity(ActiveScenes[i]);
		if (CameraEntity == nullptr)
			continue;

		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

		FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(ActiveScenes[i]);
		if (CurrentViewport == nullptr)
			continue;
		
		glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
		glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

		// FIX ME! Should be proper input system.
		glm::dvec3 MouseRay = GEOMETRY.CreateMouseRayToWorld(ENGINE.GetMouseX(), ENGINE.GetMouseY(),
															 CameraComponent.GetViewMatrix(), CameraComponent.GetProjectionMatrix(),
															 ViewportPosition, ViewportSize);

		std::vector<FEEntity*> Entities = ActiveScenes[i]->GetEntityListWith<FEVirtualUIComponent>();
		for (size_t j = 0; j < Entities.size(); j++)
		{
			FEVirtualUIComponent& VirtualUIComponent = Entities[j]->GetComponent<FEVirtualUIComponent>();

			if (VirtualUIComponent.bMouseMovePassThrough)
				VirtualUIComponent.UpdateInteractionRay(CameraEntity->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE), MouseRay);
		}
		
	}
}

FEEntity* FEVirtualUISystem::GetParentEntity(FEVirtualUI* VirtualUI)
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (size_t i = 0; i < ActiveScenes.size(); i++)
	{
		std::vector<FEEntity*> Entities = ActiveScenes[i]->GetEntityListWith<FEVirtualUIComponent>();
		for (size_t j = 0; j < Entities.size(); j++)
		{
			FEVirtualUIComponent& VirtualUIComponent = Entities[j]->GetComponent<FEVirtualUIComponent>();

			if (VirtualUIComponent.VirtualUI == VirtualUI)
				return Entities[j];
		}
	}

	return nullptr;
}

void FEVirtualUISystem::DummyRenderFunction(FEVirtualUI* VirtualUI)
{
	FEEntity* ParentEntity = VIRTUAL_UI_SYSTEM.GetParentEntity(VirtualUI);
	if (ParentEntity == nullptr)
		return;

	FEVirtualUIComponent& VirtualUIComponent = ParentEntity->GetComponent<FEVirtualUIComponent>();
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>(VirtualUIComponent.GetWidth()), static_cast<float>(VirtualUIComponent.GetHeight())));
	if (ImGui::Begin(std::string("Dummy UI##" + ParentEntity->GetObjectID()).c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNav))
	{
		ImVec2 Center = ImGui::GetWindowContentRegionMax() / 2.0f;
		
		ImGui::SetWindowFontScale(10.0f);
		ImVec2 TextSize = ImGui::CalcTextSize("Dummy UI");
		ImGui::SetCursorPos(Center - TextSize / 2.0f);
		ImGui::Text("Dummy UI");

		ImGui::End();
	}
}