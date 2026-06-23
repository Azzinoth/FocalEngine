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

	// Default values for the Basic shader's tweakable uniforms, so it renders correctly before the inspector edits them.
	VolumetricShaders.back()->UpdateUniformData("DataRangeMin", 0.0f);
	VolumetricShaders.back()->UpdateUniformData("DataRangeMax", 1.0f);
	VolumetricShaders.back()->UpdateUniformData("StepCount", 256);
	VolumetricShaders.back()->UpdateUniformData("OpacityScale", 1.0f);

	VolumetricShaders.push_back(RESOURCE_MANAGER.CreateShader("FEVolumetricShader_Cleaned", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//Volumetric//FE_Volumetric_VS.glsl").c_str()).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//Volumetric//FE_Volumetric_FS_Cloud_Like.glsl").c_str()).c_str(),
																							nullptr, nullptr, nullptr, nullptr, "391E240A020F67670C16001E"));

	// Default transfer function: blue -> cyan -> green -> yellow -> red, with opacity ramping up with the value.
	DefaultTransferFunctionColorPoints = { { 0.00f, glm::vec3(0.0f, 0.0f, 1.0f) },
										   { 0.25f, glm::vec3(0.0f, 1.0f, 1.0f) },
										   { 0.50f, glm::vec3(0.0f, 1.0f, 0.0f) },
										   { 0.75f, glm::vec3(1.0f, 1.0f, 0.0f) },
										   { 1.00f, glm::vec3(1.0f, 0.0f, 0.0f) } };
	
	DefaultTransferFunctionOpacityPoints = { { 0.0f, 0.0f },
											 { 1.0f, 1.0f } };
}

void FEVolumeSystem::InitializeTransferFunctionTexture(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())  
		return;

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
	if (VolumeComponent.VolumeMaterial == nullptr)
		return;

	if (VolumeComponent.VolumeMaterial->GetShader() == nullptr || !VolumeComponent.VolumeMaterial->GetShader()->HasUniform("TransferFunctionTexture"))
		return;

	if (VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture") != nullptr)
	{
		RESOURCE_MANAGER.DeleteFETexture(VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture"));
		VolumeComponent.VolumeMaterial->SetTextureOverride("TransferFunctionTexture", "");
	}

	std::vector<unsigned char> InitialData(TRANSFER_FUNCTION_RESOLUTION * 4, 0);
	FETexture* TransferFunctionTexture = RESOURCE_MANAGER.RawDataToFETexture(InitialData.data(), TRANSFER_FUNCTION_RESOLUTION, 1, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	if (TransferFunctionTexture == nullptr)
		return;

	EntityTransferFunctionData[Entity->GetObjectID()] = FEPerEntityTransferFunctionData();
	EntityTransferFunctionData[Entity->GetObjectID()].ColorPoints = DefaultTransferFunctionColorPoints;
	EntityTransferFunctionData[Entity->GetObjectID()].OpacityPoints = DefaultTransferFunctionOpacityPoints;

	VolumeComponent.VolumeMaterial->SetTextureOverride("TransferFunctionTexture", TransferFunctionTexture->GetObjectID());
	RESOURCE_MANAGER.SetTagInternal(TransferFunctionTexture, ENGINE_RESOURCE_TAG);

	// Linear filtering interpolates between LUT entries, clamp so the ends of the range do not wrap.
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TransferFunctionTexture->GetTextureID()));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

glm::vec3 FEVolumeSystem::EvaluateTransferFunctionColor(FEEntity* Entity, float Position)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return glm::vec3(0.0f);

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
	if (!DoesVolumeComponentHaveTransferFunction(VolumeComponent))
		return glm::vec3(0.0f);

	if (EntityTransferFunctionData.find(Entity->GetObjectID()) == EntityTransferFunctionData.end())
		return glm::vec3(0.0f);

	std::vector<FETransferFunctionColorPoint>& ColorPoints = EntityTransferFunctionData[Entity->GetObjectID()].ColorPoints;

	if (Position <= ColorPoints.front().Position)
		return ColorPoints.front().Color;

	if (Position >= ColorPoints.back().Position)
		return ColorPoints.back().Color;

	for (size_t i = 1; i < ColorPoints.size(); i++)
	{
		if (Position <= ColorPoints[i].Position)
		{
			const FETransferFunctionColorPoint& Low = ColorPoints[i - 1];
			const FETransferFunctionColorPoint& High = ColorPoints[i];
			const float Blend = (Position - Low.Position) / glm::max(High.Position - Low.Position, 1e-6f);
			return glm::mix(Low.Color, High.Color, Blend);
		}
	}

	return ColorPoints.back().Color;
}

float FEVolumeSystem::EvaluateTransferFunctionOpacity(FEEntity* Entity, float Position)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return 0.0f;

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
	if (!DoesVolumeComponentHaveTransferFunction(VolumeComponent))
		return 0.0f;

	if (EntityTransferFunctionData.find(Entity->GetObjectID()) == EntityTransferFunctionData.end())
		return 0.0f;

	std::vector<FETransferFunctionOpacityPoint>& OpacityPoints = EntityTransferFunctionData[Entity->GetObjectID()].OpacityPoints;

	if (Position <= OpacityPoints.front().Position)
		return OpacityPoints.front().Opacity;

	if (Position >= OpacityPoints.back().Position)
		return OpacityPoints.back().Opacity;

	for (size_t i = 1; i < OpacityPoints.size(); i++)
	{
		if (Position <= OpacityPoints[i].Position)
		{
			const FETransferFunctionOpacityPoint& Low = OpacityPoints[i - 1];
			const FETransferFunctionOpacityPoint& High = OpacityPoints[i];
			const float Blend = (Position - Low.Position) / glm::max(High.Position - Low.Position, 1e-6f);
			return glm::mix(Low.Opacity, High.Opacity, Blend);
		}
	}

	return OpacityPoints.back().Opacity;
}

void FEVolumeSystem::BakeTransferFunction(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return;

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
	if (!DoesVolumeComponentHaveTransferFunction(VolumeComponent) || VolumeComponent.VolumeMaterial == nullptr)
		return;

	if (VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture") == nullptr)
		return;

	FETexture* TransferFunctionTexture = VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture");
	if (TransferFunctionTexture == nullptr)
		return;

	std::vector<unsigned char> Data(TRANSFER_FUNCTION_RESOLUTION * 4);
	for (int i = 0; i < TRANSFER_FUNCTION_RESOLUTION; i++)
	{
		const float Position = static_cast<float>(i) / static_cast<float>(TRANSFER_FUNCTION_RESOLUTION - 1);
		const glm::vec3 Color = EvaluateTransferFunctionColor(Entity, Position);
		const float Opacity = EvaluateTransferFunctionOpacity(Entity, Position);

		Data[i * 4 + 0] = static_cast<unsigned char>(glm::clamp(Color.x, 0.0f, 1.0f) * 255.0f);
		Data[i * 4 + 1] = static_cast<unsigned char>(glm::clamp(Color.y, 0.0f, 1.0f) * 255.0f);
		Data[i * 4 + 2] = static_cast<unsigned char>(glm::clamp(Color.z, 0.0f, 1.0f) * 255.0f);
		Data[i * 4 + 3] = static_cast<unsigned char>(glm::clamp(Opacity, 0.0f, 1.0f) * 255.0f);
	}

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, TransferFunctionTexture->GetTextureID()));
	FE_GL_ERROR(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TRANSFER_FUNCTION_RESOLUTION, 1, GL_RGBA, GL_UNSIGNED_BYTE, Data.data()));
}

std::vector<FETransferFunctionColorPoint>& FEVolumeSystem::GetTransferFunctionColorPoints(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return DefaultTransferFunctionColorPoints;

	if (EntityTransferFunctionData.find(Entity->GetObjectID()) == EntityTransferFunctionData.end())
		return DefaultTransferFunctionColorPoints;

	return EntityTransferFunctionData[Entity->GetObjectID()].ColorPoints;
}

std::vector<FETransferFunctionOpacityPoint>& FEVolumeSystem::GetTransferFunctionOpacityPoints(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return DefaultTransferFunctionOpacityPoints;

	if (EntityTransferFunctionData.find(Entity->GetObjectID()) == EntityTransferFunctionData.end())
		return DefaultTransferFunctionOpacityPoints;

	return EntityTransferFunctionData[Entity->GetObjectID()].OpacityPoints;
}

FETexture* FEVolumeSystem::GetTransferFunctionLookupTexture(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FEVolumeComponent>())
		return nullptr;

	FEVolumeComponent& VolumeComponent = Entity->GetComponent<FEVolumeComponent>();
	if (!DoesVolumeComponentHaveTransferFunction(VolumeComponent))
		return nullptr;

	if (VolumeComponent.VolumeMaterial == nullptr)
		return nullptr;

	return VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture");
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
	VolumeComponent.ParentEntity = Entity;
	if (VolumeComponent.VolumeMaterial == nullptr && !VOLUME_SYSTEM.VolumetricShaders.empty())
	{
		FENewMaterial* DefaultStartingMaterial = new FENewMaterial("DefaultStartingMaterial");
		DefaultStartingMaterial->SetMaterialType(FEMaterialType::Volumetric);
		DefaultStartingMaterial->SetBlendMode(FEMaterialBlendMode::Additive);
		DefaultStartingMaterial->SetShader(VOLUME_SYSTEM.GetVolumetricShaders()[0]);
		VolumeComponent.SetMaterial(DefaultStartingMaterial);
	}
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
	NewVolumeComponent.VolumeMaterial = VolumeComponent.VolumeMaterial;

	NewVolumeComponent = VolumeComponent;
	NewVolumeComponent.ParentEntity = TargetEntity;
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
	Root["MaterialID"] = VolumeComponent.VolumeMaterial == nullptr ? "none" : VolumeComponent.VolumeMaterial->GetObjectID();
	
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
	if (Root.isMember("MaterialID") && Root["MaterialID"].isString())
	{
		std::string MaterialID = Root["MaterialID"].asString();
		/*if (MaterialID != "none")
			VolumeComponent.VolumeMaterial = RESOURCE_MANAGER.GetMaterial(MaterialID);*/
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
	if (CurrentVolumeComponent.VolumeMaterial == nullptr)
	{
		LOG.Add("FEVolumeSystem::RenderVolumeComponent: CurrentVolumeComponent.VolumeMaterial is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (CurrentVolumeComponent.VolumeMaterial != nullptr && VolumeComponent.VolumeMaterial->GetTextureOverride("volumeTexture") == nullptr)
		return false;

	if (CurrentVolumeComponent.VolumeMaterial != nullptr)
	{
		if (CurrentVolumeComponent.VolumeMaterial->HasUniform("DataRangeMin") && CurrentVolumeComponent.VolumeMaterial->HasUniform("DataRangeMax"))
		{
			FETexture* BoundVolumeTexture = VolumeComponent.VolumeMaterial->GetTextureOverride("volumeTexture");
			if (BoundVolumeTexture != nullptr)
			{
				// FE_FIX_ME: Should not be here. Maybe it should be available as standardized uniform in the shader. For now, we will just set it here.
				const float RawMin = BoundVolumeTexture->GetMinValue().x;
				const float RawMax = BoundVolumeTexture->GetMaxValue().x;

				float Scale = 1.0f;
				if (BoundVolumeTexture->GetInternalFormat() == GL_R16)
				{
					Scale = 65535.0f;
				}
				else if (BoundVolumeTexture->GetInternalFormat() == GL_RED)
				{
					Scale = 255.0f;
				}

				CurrentVolumeComponent.VolumeMaterial->UpdateUniformOverrideData("DataRangeMin", RawMin / Scale);
				CurrentVolumeComponent.VolumeMaterial->UpdateUniformOverrideData("DataRangeMax", RawMax / Scale);
			}
		}
		
		CurrentVolumeComponent.VolumeMaterial->Bind();
	}

	FEMesh* ScreenQuad = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	FE_GL_ERROR(glBindVertexArray(ScreenQuad->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, ScreenQuad->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	if (CurrentVolumeComponent.VolumeMaterial != nullptr)
		CurrentVolumeComponent.VolumeMaterial->UnBind();
	
	return true;
}

bool FEVolumeSystem::DoesVolumeComponentHaveTransferFunction(FEVolumeComponent& VolumeComponent)
{
	return VolumeComponent.VolumeMaterial != nullptr && VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture") != nullptr;
}