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

	ComputePointCloudShader = RESOURCE_MANAGER.CreateShader("ComputePointCloudShader",
															nullptr, nullptr,
															nullptr, nullptr,
															nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PointCloudShaders//ComputeShaders//FE_PointCloud_CS.glsl").c_str()).c_str());

	ComputePointCloudAfterRenderingShader = RESOURCE_MANAGER.CreateShader("FE_PointCloud_After_Rendering",
																		  nullptr, nullptr,
																		  nullptr, nullptr,
																		  nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PointCloudShaders//ComputeShaders//FE_PointCloud_After_Rendering_CS.glsl").c_str()).c_str());
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
	NewPointCloudComponent.SetPointCloud(PointCloudComponent.GetPointCloud());
	NewPointCloudComponent.SetGlobalColorOverride(PointCloudComponent.GetGlobalColorOverride());
	NewPointCloudComponent.SetUseGlobalColorOverride(PointCloudComponent.IsUsingGlobalColorOverride());
	NewPointCloudComponent.SetVisibility(PointCloudComponent.IsVisible());

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

void FEPointCloudSystem::RenderStandard(FEEntity* Entity, FEEntity* Camera)
{
	if (Entity == nullptr || Camera == nullptr)
		return;

	if (!Entity->HasComponent<FEPointCloudComponent>())
		return;

	RenderPointCloudComponent(Entity->GetComponent<FETransformComponent>(), Entity->GetComponent<FEPointCloudComponent>(), Camera);
}

bool FEPointCloudSystem::RenderPointCloudComponent(FETransformComponent& TransformComponent, FEPointCloudComponent& PointCloudComponent, FEEntity* Camera)
{
	if (Camera == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderPointCloudComponent: Camera is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FEPointCloud* PointCloud = PointCloudComponent.GetPointCloud();
	if (PointCloud == nullptr || Camera == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderPointCloudComponent: PointCloud is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!PointCloud->IsAdvancedRenderingEnabled())
	{
		if (StandardPointCloudShader == nullptr)
		{
			LOG.Add("FEPointCloudSystem::RenderPointCloudComponent: StandardPointCloudShader is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
			return false;
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
			StandardPointCloudShader->UpdateUniformData("GlobalColorOverride", PointCloudComponent.GetGlobalColorOverride());
		}
		else
		{
			StandardPointCloudShader->UpdateUniformData("bUseGlobalColorOverride", false);
			StandardPointCloudShader->UpdateUniformData("GlobalColorOverride", glm::vec3(0.0f));
		}
		StandardPointCloudShader->LoadUniformsDataToGPU();

		FE_GL_ERROR(glBindVertexArray(PointCloud->GetVaoID()));
		FE_GL_ERROR(glDrawArrays(GL_POINTS, 0, static_cast<GLuint>(PointCloud->GetPointCount())));

		StandardPointCloudShader->Stop();

		return true;
	}

	return false;
}

#include "../../Renderer/FERenderer.h"
bool FEPointCloudSystem::RenderWithComputeShaders(FETransformComponent& TransformComponent, FEPointCloudComponent& PointCloudComponent, FEEntity* Camera)
{
	if (Camera == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: Camera is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!Camera->HasComponent<FECameraComponent>())
	{
		LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: Camera does not have FECameraComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraRenderingData* CameraRenderingData = RENDERER.GetCameraRenderingData(Camera);
	if (CameraRenderingData == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: CameraRenderingData is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FEPointCloud* PointCloud = PointCloudComponent.GetPointCloud();
	if (PointCloud == nullptr || Camera == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: PointCloud is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!PointCloud->IsAdvancedRenderingEnabled())
	{
		LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: PointCloud is not set to advanced rendering", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}
	
	if (ComputePointCloudShader == nullptr || ComputePointCloudAfterRenderingShader == nullptr)
	{
		LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: ComputePointCloudShader or ComputePointCloudAfterRenderingShader is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
	int ScreenWidth = CameraComponent.GetRenderTargetWidth();
	int ScreenHeight = CameraComponent.GetRenderTargetHeight();

	POINT_CLOUD_SYSTEM.ComputePointCloudShader->Start();
	POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("FEWorldMatrix", TransformComponent.GetWorldMatrix());
	POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("FEViewMatrix", CameraComponent.GetViewMatrix());
	POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("FEProjectionMatrix", CameraComponent.GetProjectionMatrix());
	POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("ScreenWidth", ScreenWidth);
	POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("ScreenHeight", ScreenHeight);
	if (PointCloudComponent.IsUsingGlobalColorOverride())
	{
		POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("bUseGlobalColorOverride", true);
		POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("GlobalColorOverride", PointCloudComponent.GetGlobalColorOverride());
	}
	else
	{
		POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("bUseGlobalColorOverride", false);
		POINT_CLOUD_SYSTEM.ComputePointCloudShader->UpdateUniformData("GlobalColorOverride", glm::vec3(0.0f));
	}

	POINT_CLOUD_SYSTEM.ComputePointCloudShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PointCloud->ComputeShaderBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PointCloud->ComputeShaderBuffer));

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CameraRenderingData->PointCloud64bitFrameBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CameraRenderingData->PointCloud64bitFrameBuffer));

	// If we have more points than the maximum points per buffer, we will run the compute shader multiple times.
	if (PointCloud->PointCount > FEPointCloud::MaxPointsPerBuffer)
	{
		POINT_CLOUD_SYSTEM.ComputePointCloudShader->Dispatch(static_cast<GLuint>((FEPointCloud::MaxPointsPerBuffer / 1024) + 1), 1, 1);
		size_t BufferIndex = 0;

		for (size_t i = FEPointCloud::MaxPointsPerBuffer; i < PointCloud->PointCount; i += FEPointCloud::MaxPointsPerBuffer)
		{
			if (BufferIndex >= PointCloud->ComputeShaderBuffers.size())
			{
				LOG.Add("FEPointCloudSystem::RenderWithComputeShaders: BufferIndex is out of range", "FE_LOG_RENDERING", FE_LOG_ERROR);
			}
			else
			{
				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PointCloud->ComputeShaderBuffers[BufferIndex]));
				FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PointCloud->ComputeShaderBuffers[BufferIndex]));
				BufferIndex++;

				// Calculate the number of points for the current buffer
				size_t NumberOfPoints = std::min(FEPointCloud::MaxPointsPerBuffer, PointCloud->PointCount - i);
				POINT_CLOUD_SYSTEM.ComputePointCloudShader->Dispatch(static_cast<GLuint>((NumberOfPoints / 1024) + 1), 1, 1);
			}
		}
	}
	else
	{
		POINT_CLOUD_SYSTEM.ComputePointCloudShader->Dispatch(static_cast<GLuint>((PointCloud->GetPointCount() / 1024) + 1), 1, 1);
	}

	FE_GL_ERROR(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
	return true;
}

bool FEPointCloudSystem::FuseComputeRenderedToFramebuffer(FEEntity* Camera)
{
	if (Camera == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: Camera is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!Camera->HasComponent<FECameraComponent>())
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: Camera does not have FECameraComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraRenderingData* CameraRenderingData = RENDERER.GetCameraRenderingData(Camera);
	if (CameraRenderingData == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: CameraRenderingData is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (ComputePointCloudShader == nullptr || ComputePointCloudAfterRenderingShader == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: ComputePointCloudShader or ComputePointCloudAfterRenderingShader is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
	int ScreenWidth = CameraComponent.GetRenderTargetWidth();
	int ScreenHeight = CameraComponent.GetRenderTargetHeight();

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->Start();
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("ScreenWidth", ScreenWidth);
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("ScreenHeight", ScreenHeight);
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("NearPlane", CameraComponent.GetNearPlane());
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("FarPlane", CameraComponent.GetFarPlane());

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindImageTexture(0, CameraRenderingData->PointCloudIntermediateFrameBuffer->GetColorAttachment()->GetTextureID(), static_cast<GLint>(0), GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F));
	FE_GL_ERROR(glBindImageTexture(1, CameraRenderingData->PointCloudIntermediateFrameBuffer->GetDepthAttachment()->GetTextureID(), static_cast<GLint>(0), GL_FALSE, 0, GL_READ_WRITE, GL_R32F));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CameraRenderingData->PointCloud64bitFrameBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CameraRenderingData->PointCloud64bitFrameBuffer));

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->Dispatch(static_cast<unsigned>(ceil(float(ScreenWidth) / 32.0f)), static_cast<unsigned>(ceil(float(ScreenHeight) / 32.0f)), 1);
	FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->Stop();
	// ************ Extract color part from custom frame buffer END ************

	// We are rendering point clouds using same near and far plane values as main camera.
	return RENDERER.FuseFrameBufferDataAndCameraData(CameraRenderingData->PointCloudIntermediateFrameBuffer, Camera,
												   CameraComponent.GetNearPlane(), CameraComponent.GetFarPlane(),
												   glm::vec3(1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f),
												   glm::vec4(0.5f, 0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
}

bool FEPointCloudSystem::FuseComputeRenderedToFramebuffer(FEEntity* CameraUsed, FEFramebuffer* TargetFrameBuffer)
{
	if (TargetFrameBuffer == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: TargetFrameBuffer is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (CameraUsed == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: Camera is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!CameraUsed->HasComponent<FECameraComponent>())
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: Camera does not have FECameraComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraRenderingData* CameraRenderingData = RENDERER.GetCameraRenderingData(CameraUsed);
	if (CameraRenderingData == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: CameraRenderingData is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (CameraRenderingData->PointCloudIntermediateFrameBuffer == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: CameraRenderingData->PointCloudIntermediateFrameBuffer is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (TargetFrameBuffer->GetColorAttachment() == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: TargetFrameBuffer does not have color attachment", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (TargetFrameBuffer->GetWidth() != CameraRenderingData->PointCloudIntermediateFrameBuffer->GetWidth() ||
		TargetFrameBuffer->GetHeight() != CameraRenderingData->PointCloudIntermediateFrameBuffer->GetHeight())
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: TargetFrameBuffer size does not match with CameraRenderingData->PointCloudIntermediateFrameBuffer size", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (ComputePointCloudShader == nullptr || ComputePointCloudAfterRenderingShader == nullptr)
	{
		LOG.Add("FEPointCloudSystem::FuseComputeRenderedToFramebuffer: ComputePointCloudShader or ComputePointCloudAfterRenderingShader is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CameraComponent = CameraUsed->GetComponent<FECameraComponent>();
	int ScreenWidth = CameraComponent.GetRenderTargetWidth();
	int ScreenHeight = CameraComponent.GetRenderTargetHeight();

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->Start();
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("ScreenWidth", ScreenWidth);
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("ScreenHeight", ScreenHeight);
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("NearPlane", CameraComponent.GetNearPlane());
	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->UpdateUniformData("FarPlane", CameraComponent.GetFarPlane());

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindImageTexture(0, CameraRenderingData->PointCloudIntermediateFrameBuffer->GetColorAttachment()->GetTextureID(), static_cast<GLint>(0), GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F));
	FE_GL_ERROR(glBindImageTexture(1, CameraRenderingData->PointCloudIntermediateFrameBuffer->GetDepthAttachment()->GetTextureID(), static_cast<GLint>(0), GL_FALSE, 0, GL_READ_WRITE, GL_R32F));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, CameraRenderingData->PointCloud64bitFrameBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CameraRenderingData->PointCloud64bitFrameBuffer));

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->Dispatch(static_cast<unsigned>(ceil(float(ScreenWidth) / 32.0f)), static_cast<unsigned>(ceil(float(ScreenHeight) / 32.0f)), 1);
	FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));

	POINT_CLOUD_SYSTEM.ComputePointCloudAfterRenderingShader->Stop();
	// ************ Extract color part from custom frame buffer END ************

	return RENDERER.FuseTwoFrameBuffers(CameraRenderingData->PointCloudIntermediateFrameBuffer, CameraComponent.GetNearPlane(), CameraComponent.GetFarPlane(),
										TargetFrameBuffer, CameraComponent.GetNearPlane(), CameraComponent.GetFarPlane(), TargetFrameBuffer);
}

bool FEPointCloudSystem::SetAdvancedRendering(FEPointCloud* PointCloud, bool bUseAdvancedRendering)
{
	if (PointCloud == nullptr)
		return false;

	if (PointCloud->GetPointCount() == 0)
		return true;

	std::vector<FEPointCloudVertex> RawPointCloudData = PointCloud->GetRawData();
	if (RawPointCloudData.empty())
	{
		LOG.Add("FEPointCloudSystem::SetAdvancedRendering: RawPointCloudData is empty", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!PointCloud->IsAdvancedRenderingEnabled())
	{
		if (PointCloud->VaoID != GLuint(-1))
		{
			FE_GL_ERROR(glDeleteVertexArrays(1, &PointCloud->VaoID));
			PointCloud->VaoID = GLuint(-1);
		}
			
		if (PointCloud->VboID != GLuint(-1))
		{
			FE_GL_ERROR(glDeleteBuffers(1, &PointCloud->VboID));
			PointCloud->VboID = GLuint(-1);
		}

		FE_GL_ERROR(glGenBuffers(1, &PointCloud->ComputeShaderBuffer));
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, PointCloud->ComputeShaderBuffer));
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, PointCloud->ComputeShaderBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(FEPointCloudVertex) * PointCloud->PointCount, RawPointCloudData.data(), GL_DYNAMIC_DRAW));

		return true;
	}
	else
	{
		if (PointCloud->ComputeShaderBuffer != GLuint(-1))
		{
			FE_GL_ERROR(glDeleteBuffers(1, &PointCloud->ComputeShaderBuffer));
			PointCloud->ComputeShaderBuffer = GLuint(-1);
		}

		FE_GL_ERROR(glGenBuffers(1, &PointCloud->VboID));

		// Bind and upload vertex data to the VBO.
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PointCloud->VboID));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, sizeof(FEPointCloudVertex) * PointCloud->PointCount, RawPointCloudData.data(), GL_STATIC_DRAW));

		FE_GL_ERROR(glGenVertexArrays(1, &PointCloud->VaoID));

		// Bind and link VAO and VBO.
		FE_GL_ERROR(glBindVertexArray(PointCloud->VaoID));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, PointCloud->VboID));

		FE_GL_ERROR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FEPointCloudVertex), (void*)0));
		FE_GL_ERROR(glEnableVertexAttribArray(0));

		FE_GL_ERROR(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(FEPointCloudVertex), (void*)(3 * sizeof(float))));
		FE_GL_ERROR(glEnableVertexAttribArray(1));

		return true;
	}

	return false;
}