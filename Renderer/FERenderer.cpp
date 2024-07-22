#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::Instance = nullptr;

FERenderer::FERenderer()
{
}

void FERenderer::Init()
{
	if (bSimplifiedRendering)
	{
		// Because of VR
		glGenBuffers(1, &UniformBufferForLights);
		glBindBuffer(GL_UNIFORM_BUFFER, UniformBufferForLights);
		glBufferData(GL_UNIFORM_BUFFER, FE_MAX_LIGHTS * UBufferForLightSize, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UniformBufferCount++, UniformBufferForLights, 0, FE_MAX_LIGHTS * UBufferForLightSize);

		glGenBuffers(1, &UniformBufferForDirectionalLight);
		glBindBuffer(GL_UNIFORM_BUFFER, UniformBufferForDirectionalLight);
		glBufferData(GL_UNIFORM_BUFFER, UBufferForDirectionalLightSize, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UniformBufferCount++, UniformBufferForDirectionalLight, 0, UBufferForDirectionalLightSize);
	}
	else
	{
		glGenBuffers(1, &UniformBufferForLights);
		glBindBuffer(GL_UNIFORM_BUFFER, UniformBufferForLights);
		glBufferData(GL_UNIFORM_BUFFER, FE_MAX_LIGHTS * UBufferForLightSize, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UniformBufferCount++, UniformBufferForLights, 0, FE_MAX_LIGHTS * UBufferForLightSize);

		glGenBuffers(1, &UniformBufferForDirectionalLight);
		glBindBuffer(GL_UNIFORM_BUFFER, UniformBufferForDirectionalLight);
		glBufferData(GL_UNIFORM_BUFFER, UBufferForDirectionalLightSize, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, UniformBufferCount++, UniformBufferForDirectionalLight, 0, UBufferForDirectionalLightSize);

		// Instanced lines
		LinesBuffer.resize(FE_MAX_LINES);

		const float QuadVertices[] = {
			0.0f,  -0.5f,  0.0f,
			1.0f,  -0.5f,  1.0f,
			1.0f,  0.5f,   1.0f,

			0.0f,  -0.5f,  0.0f,
			1.0f,  0.5f,   1.0f,
			0.0f,  0.5f,   0.0f,
		};
		glGenVertexArrays(1, &InstancedLineVAO);
		glBindVertexArray(InstancedLineVAO);

		unsigned int QuadVBO;
		glGenBuffers(1, &QuadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		glGenBuffers(1, &InstancedLineBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, InstancedLineBuffer);
		glBufferData(GL_ARRAY_BUFFER, LinesBuffer.size() * sizeof(FELine), LinesBuffer.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), static_cast<void*>(nullptr));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(6 * sizeof(float)));
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(9 * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);

		glBindVertexArray(0);

		FrustumCullingShader = RESOURCE_MANAGER.CreateShader("FE_FrustumCullingShader",
															 nullptr, nullptr,
															 nullptr, nullptr,
															 nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//ComputeShaders//FE_FrustumCulling_CS.glsl").c_str()).c_str());

		FE_GL_ERROR(glGenBuffers(1, &FrustumInfoBuffer));
		FE_GL_ERROR(glGenBuffers(1, &CullingLODCountersBuffer));

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, CullingLODCountersBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * 40, nullptr, GL_DYNAMIC_DRAW));

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FrustumInfoBuffer));

		std::vector<float> FrustumData;
		for (size_t i = 0; i < 32; i++)
		{
			FrustumData.push_back(0.0);
		}

		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * (32), FrustumData.data(), GL_DYNAMIC_DRAW));


		ComputeTextureCopy = RESOURCE_MANAGER.CreateShader("FE_ComputeTextureCopy",
														   nullptr, nullptr,
														   nullptr, nullptr,
														   nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//ComputeShaders//FE_ComputeTextureCopy_CS.glsl").c_str()).c_str());


		ComputeDepthPyramidDownSample = RESOURCE_MANAGER.CreateShader("FE_ComputeDepthPyramidDownSample",
																	  nullptr, nullptr,
																	  nullptr, nullptr,
																	  nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//ComputeShaders//FE_ComputeDepthPyramidDownSample_CS.glsl").c_str()).c_str());

		ComputeDepthPyramidDownSample->UpdateParameterData("scaleDownBy", 2);
	}
}

void FERenderer::StandardFBInit(const int WindowWidth, const int WindowHeight)
{
	SceneToTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);

	if (bSimplifiedRendering)
		return;

	GBuffer = new FEGBuffer(SceneToTextureFB);
	SSAO = new FESSAO(SceneToTextureFB);
	DebugOutputTextures["SSAO FrameBuffer"] = []() { return RENDERER.SSAO->FB->GetColorAttachment(); };

	DebugOutputTextures["albedoRenderTarget"] = []() { return RENDERER.GBuffer->Albedo; };
	DebugOutputTextures["positionsRenderTarget"] = []() { return RENDERER.GBuffer->Positions; };
	DebugOutputTextures["normalsRenderTarget"] = []() { return RENDERER.GBuffer->Normals; };
	DebugOutputTextures["materialPropertiesRenderTarget"] = []() { return RENDERER.GBuffer->MaterialProperties; };
	DebugOutputTextures["shaderPropertiesRenderTarget"] = []() { return RENDERER.GBuffer->ShaderProperties; };

	DebugOutputTextures["CSM0"] = []() { return RENDERER.CSM0; };
	DebugOutputTextures["CSM1"] = []() { return RENDERER.CSM1; };
	DebugOutputTextures["CSM2"] = []() { return RENDERER.CSM2; };
	DebugOutputTextures["CSM3"] = []() { return RENDERER.CSM3; };

	DepthPyramid = RESOURCE_MANAGER.CreateTexture();
	RESOURCE_MANAGER.Textures.erase(DepthPyramid->GetObjectID());

	DepthPyramid->Bind();
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	const int MaxDimention = std::max(WindowWidth, WindowHeight);
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_R32F, WindowWidth, WindowHeight));
	DepthPyramid->Width = WindowWidth;
	DepthPyramid->Height = WindowHeight;
}

void FERenderer::LoadStandardParams(FEShader* Shader, FEMaterial* Material, const FETransformComponent* Transform, FEEntity* ForceCamera, const bool IsReceivingShadows, const bool IsUniformLighting)
{
	static int FETextureBindingsUniformLocationsHash = static_cast<int>(std::hash<std::string>{}("textureBindings[0]"));
	static int FETextureChannelsBindingsUniformLocationsHash = static_cast<int>(std::hash<std::string>{}("textureChannels[0]"));

	if (Material != nullptr)
	{
		if (Shader->bMaterialTexturesList)
		{
			Shader->LoadIntArray(FETextureBindingsUniformLocationsHash, Material->TextureBindings.data(), Material->TextureBindings.size());
			Shader->LoadIntArray(FETextureChannelsBindingsUniformLocationsHash, Material->TextureChannels.data(), Material->TextureChannels.size());
		}
	}

	//  FIX ME!
	FEEntity* CameraEntityToUse = ForceCamera;
	if (ForceCamera == nullptr)
	{
		CameraEntityToUse = TryToGetLastUsedCameraEntity();
		if (CameraEntityToUse == nullptr)
			return;
	}
	FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = CameraEntityToUse->GetComponent<FETransformComponent>();

	if (Shader->GetParameter("FEWorldMatrix") != nullptr)
		Shader->UpdateParameterData("FEWorldMatrix", Transform->GetWorldMatrix());

	if (Shader->GetParameter("FEViewMatrix") != nullptr)
		Shader->UpdateParameterData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());

	if (Shader->GetParameter("FEProjectionMatrix") != nullptr)
		Shader->UpdateParameterData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());

	if (Shader->GetParameter("FEPVMMatrix") != nullptr)
		Shader->UpdateParameterData("FEPVMMatrix", CurrentCameraComponent.GetProjectionMatrix() * CurrentCameraComponent.GetViewMatrix() * Transform->GetWorldMatrix());

	if (Shader->GetParameter("FECameraPosition") != nullptr)
		Shader->UpdateParameterData("FECameraPosition", CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE));

	if (Shader->GetParameter("FEGamma") != nullptr)
		Shader->UpdateParameterData("FEGamma", CurrentCameraComponent.GetGamma());

	if (Shader->GetParameter("FEExposure") != nullptr)
		Shader->UpdateParameterData("FEExposure", CurrentCameraComponent.GetExposure());

	if (Shader->GetParameter("FEReceiveShadows") != nullptr)
		Shader->UpdateParameterData("FEReceiveShadows", IsReceivingShadows);

	if (Shader->GetParameter("FEUniformLighting") != nullptr)
		Shader->UpdateParameterData("FEUniformLighting", IsUniformLighting);

	if (Material != nullptr)
	{
		if (Shader->GetParameter("FEAOIntensity") != nullptr)
			Shader->UpdateParameterData("FEAOIntensity", Material->GetAmbientOcclusionIntensity());

		if (Shader->GetParameter("FEAOMapIntensity") != nullptr)
			Shader->UpdateParameterData("FEAOMapIntensity", Material->GetAmbientOcclusionMapIntensity());

		if (Shader->GetParameter("FENormalMapIntensity") != nullptr)
			Shader->UpdateParameterData("FENormalMapIntensity", Material->GetNormalMapIntensity());

		if (Shader->GetParameter("FERoughness") != nullptr)
			Shader->UpdateParameterData("FERoughness", Material->Roughness);

		if (Shader->GetParameter("FERoughnessMapIntensity") != nullptr)
			Shader->UpdateParameterData("FERoughnessMapIntensity", Material->GetRoughnessMapIntensity());

		if (Shader->GetParameter("FEMetalness") != nullptr)
			Shader->UpdateParameterData("FEMetalness", Material->Metalness);

		if (Shader->GetParameter("FEMetalnessMapIntensity") != nullptr)
			Shader->UpdateParameterData("FEMetalnessMapIntensity", Material->GetMetalnessMapIntensity());

		if (Shader->GetParameter("FETiling") != nullptr)
			Shader->UpdateParameterData("FETiling", Material->GetTiling());

		if (Shader->GetParameter("compactMaterialPacking") != nullptr)
			Shader->UpdateParameterData("compactMaterialPacking", Material->IsCompackPacking());
	}
}

void FERenderer::LoadStandardParams(FEShader* Shader, const bool IsReceivingShadows, FEEntity* ForceCamera, const bool IsUniformLighting)
{
	//  FIX ME!
	FEEntity* CameraEntityToUse = ForceCamera;
	if (ForceCamera == nullptr)
	{
		CameraEntityToUse = TryToGetLastUsedCameraEntity();
		if (CameraEntityToUse == nullptr)
			return;
	}
	//  FIX ME!
	FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = CameraEntityToUse->GetComponent<FETransformComponent>();

	if (Shader->GetParameter("FEViewMatrix") != nullptr)
		Shader->UpdateParameterData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());

	if (Shader->GetParameter("FEProjectionMatrix") != nullptr)
		Shader->UpdateParameterData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());

	if (Shader->GetParameter("FECameraPosition") != nullptr)
		Shader->UpdateParameterData("FECameraPosition", CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE));

	if (Shader->GetParameter("FEGamma") != nullptr)
		Shader->UpdateParameterData("FEGamma", CurrentCameraComponent.GetGamma());

	if (Shader->GetParameter("FEExposure") != nullptr)
		Shader->UpdateParameterData("FEExposure", CurrentCameraComponent.GetExposure());

	if (Shader->GetParameter("FEReceiveShadows") != nullptr)
		Shader->UpdateParameterData("FEReceiveShadows", IsReceivingShadows);

	if (Shader->GetParameter("FEUniformLighting") != nullptr)
		Shader->UpdateParameterData("FEUniformLighting", IsUniformLighting);
}

void FERenderer::AddPostProcess(FEPostProcess* NewPostProcess, const bool NoProcessing)
{
	PostProcessEffects.push_back(NewPostProcess);

	if (NoProcessing)
		return;

	for (size_t i = 0; i < PostProcessEffects.back()->Stages.size(); i++)
	{
		PostProcessEffects.back()->Stages[i]->InTexture.resize(PostProcessEffects.back()->Stages[i]->InTextureSource.size());
		//to-do: change when out texture could be different resolution or/and format.
		//#fix
		if (i == PostProcessEffects.back()->Stages.size() - 1)
		{
			PostProcessEffects.back()->Stages[i]->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(SceneToTextureFB->GetColorAttachment());
		}
		else
		{
			const int FinalW = PostProcessEffects.back()->ScreenWidth;
			const int FinalH = PostProcessEffects.back()->ScreenHeight;
			PostProcessEffects.back()->Stages[i]->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(SceneToTextureFB->GetColorAttachment(), FinalW, FinalH);
		}

		PostProcessEffects.back()->TexturesToDelete.push_back(PostProcessEffects.back()->Stages[i]->OutTexture);
	}
}

void FERenderer::LoadUniformBlocks(FEScene* CurrentScene)
{
	std::vector<FELightShaderInfo> Info;
	Info.resize(FE_MAX_LIGHTS);

	// direction light information for shaders
	FEDirectionalLightShaderInfo DirectionalLightInfo;

	int Index = 0;
	std::vector< std::string> LightsIDList = CurrentScene->GetEntityIDListWith<FELightComponent>();
	for (size_t i = 0; i < LightsIDList.size(); i++)
	{
		FEEntity* LightEntity = CurrentScene->GetEntity(LightsIDList[i]);
		FETransformComponent& TransformComponent = LightEntity->GetComponent<FETransformComponent>();
		FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

		if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
		{
			DirectionalLightInfo.Position = glm::vec4(TransformComponent.GetPosition(), 0.0f);
			DirectionalLightInfo.Color = glm::vec4(LightComponent.GetColor() * LightComponent.GetIntensity(), 0.0f);
			DirectionalLightInfo.Direction = glm::vec4(LIGHT_SYSTEM.GetDirection(LightEntity), 0.0f);
			DirectionalLightInfo.CSM0 = LightComponent.CascadeData[0].ProjectionMat * LightComponent.CascadeData[0].ViewMat;
			DirectionalLightInfo.CSM1 = LightComponent.CascadeData[1].ProjectionMat * LightComponent.CascadeData[1].ViewMat;
			DirectionalLightInfo.CSM2 = LightComponent.CascadeData[2].ProjectionMat * LightComponent.CascadeData[2].ViewMat;
			DirectionalLightInfo.CSM3 = LightComponent.CascadeData[3].ProjectionMat * LightComponent.CascadeData[3].ViewMat;
			DirectionalLightInfo.CSMSizes = glm::vec4(LightComponent.CascadeData[0].Size, LightComponent.CascadeData[1].Size, LightComponent.CascadeData[2].Size, LightComponent.CascadeData[3].Size);
			DirectionalLightInfo.ActiveCascades = LightComponent.ActiveCascades;
			DirectionalLightInfo.BiasFixed = LightComponent.ShadowBias;
			if (!LightComponent.bStaticShadowBias)
				DirectionalLightInfo.BiasFixed = -1.0f;
			DirectionalLightInfo.BiasVariableIntensity = LightComponent.ShadowBiasVariableIntensity;
			DirectionalLightInfo.Intensity = LightComponent.GetIntensity();
		}
		else if (LightComponent.GetType() == FE_SPOT_LIGHT)
		{
			Info[Index].TypeAndAngles = glm::vec4(LightComponent.GetType(),
												  glm::cos(glm::radians(LightComponent.GetSpotAngle())),
												  glm::cos(glm::radians(LightComponent.GetSpotAngleOuter())),
												  0.0f);

			Info[Index].Direction = glm::vec4(LIGHT_SYSTEM.GetDirection(LightEntity), 0.0f);
		}
		else if (LightComponent.GetType() == FE_POINT_LIGHT)
		{
			Info[Index].TypeAndAngles = glm::vec4(LightComponent.GetType(), 0.0f, 0.0f, 0.0f);
		}

		Info[Index].Position = glm::vec4(TransformComponent.GetPosition(), 0.0f);
		Info[Index].Color = glm::vec4(LightComponent.GetColor() * LightComponent.GetIntensity(), 0.0f);

		Index++;
	}

	//#fix only standardShaders uniforms buffers are filled.
	static int LightInfoHash = static_cast<int>(std::hash<std::string>{}("lightInfo"));
	static int DirectionalLightInfoHash = static_cast<int>(std::hash<std::string>{}("directionalLightInfo"));
	const std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetStandardShadersList();
	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		FEShader* Shader = RESOURCE_MANAGER.GetShader(ShaderList[i]);
		auto IteratorBlock = Shader->BlockUniforms.begin();
		while (IteratorBlock != Shader->BlockUniforms.end())
		{
			if (IteratorBlock->first == LightInfoHash)
			{
				// if shader uniform block was not asigned yet.
				if (IteratorBlock->second == GL_INVALID_INDEX)
					IteratorBlock->second = UniformBufferForLights;
				// adding 4 because vec3 in shader buffer will occupy 16 bytes not 12.
				const size_t SizeOfFELightShaderInfo = sizeof(FELightShaderInfo);// +4;
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, IteratorBlock->second));

				//FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, SizeOfFELightShaderInfo * Info.size(), &Info));
				for (size_t j = 0; j < Info.size(); j++)
				{
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, j * SizeOfFELightShaderInfo, SizeOfFELightShaderInfo, &Info[j]));
				}

				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}
			else if (IteratorBlock->first == DirectionalLightInfoHash)
			{
				// if shader uniform block was not assigned yet.
				if (IteratorBlock->second == GL_INVALID_INDEX)
					IteratorBlock->second = UniformBufferForDirectionalLight;

				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, IteratorBlock->second));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, 384, &DirectionalLightInfo));
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}

			IteratorBlock++;
		}
	}
}

void FERenderer::RenderGameModelComponentWithInstanced(FEEntity* Entity, float** Frustum, FEEntity* ForceCamera, bool bShadowMap, bool bReloadUniformBlocks)
{
	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>() || !Entity->HasComponent<FEInstancedComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	if (bReloadUniformBlocks)
		LoadUniformBlocks(Entity->ParentScene);

	GPUCulling(TransformComponent, GameModelComponent, InstancedComponent);

	FEGameModel* CurrentGameModel = GameModelComponent.GameModel;
	FEShader* OriginalShader = CurrentGameModel->GetMaterial()->Shader;
	if (OriginalShader->GetName() == "FEPBRShader")
	{
		if (ShaderToForce)
		{
			CurrentGameModel->GetMaterial()->Shader = ShaderToForce;
		}
		else
		{
			CurrentGameModel->GetMaterial()->Shader = RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
		}
	}

	CurrentGameModel->GetMaterial()->Bind();
	LoadStandardParams(CurrentGameModel->GetMaterial()->Shader, CurrentGameModel->Material, &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	CurrentGameModel->GetMaterial()->Shader->LoadDataToGPU();

	INSTANCED_RENDERING_SYSTEM.Render(TransformComponent, GameModelComponent, InstancedComponent);

	CurrentGameModel->GetMaterial()->UnBind();
	if (OriginalShader->GetName() == "FEPBRShader")
	{
		CurrentGameModel->GetMaterial()->Shader = OriginalShader;
		if (CurrentGameModel->GetBillboardMaterial() != nullptr)
			CurrentGameModel->GetBillboardMaterial()->Shader = OriginalShader;
	}

	// Billboards part
	if (CurrentGameModel->GetBillboardMaterial() != nullptr)
	{
		FEMaterial* RegularBillboardMaterial = CurrentGameModel->GetBillboardMaterial();
		if (bShadowMap)
		{
			ShadowMapMaterialInstanced->SetAlbedoMap(RegularBillboardMaterial->GetAlbedoMap());
			CurrentGameModel->SetBillboardMaterial(ShadowMapMaterialInstanced);
		}

		OriginalShader = CurrentGameModel->GetMaterial()->Shader;
		if (OriginalShader->GetName() == "FEPBRShader")
		{
			if (ShaderToForce)
			{
				CurrentGameModel->GetBillboardMaterial()->Shader = ShaderToForce;
			}
			else
			{
				CurrentGameModel->GetBillboardMaterial()->Shader = RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
			}
		}

		CurrentGameModel->GetBillboardMaterial()->Bind();
		LoadStandardParams(CurrentGameModel->GetBillboardMaterial()->Shader, CurrentGameModel->GetBillboardMaterial(), &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
		CurrentGameModel->GetBillboardMaterial()->Shader->LoadDataToGPU();

		INSTANCED_RENDERING_SYSTEM.RenderOnlyBillbords(GameModelComponent, InstancedComponent);

		CurrentGameModel->GetBillboardMaterial()->UnBind();
		if (OriginalShader->GetName() == "FEPBRShader")
			CurrentGameModel->GetBillboardMaterial()->Shader = OriginalShader;

		if (bShadowMap)
		{
			CurrentGameModel->SetBillboardMaterial(RegularBillboardMaterial);
		}
	}
}

void FERenderer::SimplifiedRender(FEScene* CurrentScene, FEBasicCamera* CurrentCamera)
{
	if (CurrentScene == nullptr || CurrentCamera == nullptr)
		return;

	CurrentCamera->UpdateFrustumPlanes();

	SceneToTextureFB->Bind();
	//glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	
	if (bClearActiveInSimplifiedRendering)
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// No instanced rendering for now.
	entt::basic_group GameModelGroup = CurrentScene->Registry.group<FEGameModelComponent>(entt::get<FETransformComponent>, entt::exclude<FEInstancedComponent>);
	for (entt::entity CurrentEnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(CurrentEnTTEntity);

		FEEntity* CurrentEntity = CurrentScene->GetEntityByEnTT(CurrentEnTTEntity);
		if (CurrentEntity == nullptr)
			continue;

		if (GameModelComponent.IsVisible() && GameModelComponent.IsPostprocessApplied())
		{
			// FIX ME! CAMERA
			RenderGameModelComponentForward(CurrentEntity, nullptr);
		}
	}

	SceneToTextureFB->UnBind();
	FinalScene = SceneToTextureFB->GetColorAttachment();
	FinalScene->Bind();

	// ********* RENDER FRAME BUFFER TO SCREEN *********
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	ScreenQuadShader->Start();
	// FIX ME! CAMERA
	LoadStandardParams(ScreenQuadShader, true);
	ScreenQuadShader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	ScreenQuadShader->Stop();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	// ********* RENDER FRAME BUFFER TO SCREEN END *********

	FinalScene->UnBind();
}

void FERenderer::Render(FEScene* CurrentScene, FEBasicCamera* CurrentCamera)
{
	if (CurrentScene == nullptr || CurrentCamera == nullptr)
		return;

	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCameraEntity(CurrentScene);
	if (MainCameraEntity == nullptr)
		return;

	//  FIX ME!
	this->CurrentScene = CurrentScene;
	LastRenderedSceneID = CurrentScene->GetObjectID();
	LastRenderedCameraID = MainCameraEntity->GetObjectID();

	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();

	LastRenderedResult = nullptr;

	if (bVRActive)
		return;

	if (bSimplifiedRendering)
	{
		SimplifiedRender(CurrentScene, CurrentCamera);
		return;
	}

	CurrentCameraComponent.UpdateFrustumPlanes();
	CurrentCamera->UpdateFrustumPlanes();

	LastTestTime = TestTime;
	TestTime = 0.0f;

	// there is only 1 directional light, sun.
	// and we need to set correct light position
	//#fix it should update view matrices for each cascade!

	std::vector< std::string> LightsIDList = CurrentScene->GetEntityIDListWith<FELightComponent>();
	for (size_t i = 0; i < LightsIDList.size(); i++)
	{	
		FEEntity* LightEntity = CurrentScene->GetEntity(LightsIDList[i]);
		FETransformComponent& TransformComponent = LightEntity->GetComponent<FETransformComponent>();
		FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

		if (LightComponent.GetType() != FE_DIRECTIONAL_LIGHT)
			continue;

		if (LightComponent.IsCastShadows())
		{
			LIGHT_SYSTEM.UpdateCascades(LightEntity, CurrentCameraComponent.GetFOV(), CurrentCameraComponent.GetAspectRatio(),
										CurrentCameraComponent.GetNearPlane(), CurrentCameraComponent.GetFarPlane(),
										CurrentCameraComponent.GetViewMatrix(), CurrentCameraComponent.GetForward(),
										CurrentCameraComponent.GetRight(), CurrentCameraComponent.GetUp());
		}
	}

	LoadUniformBlocks(CurrentScene);

	// ********* GENERATE SHADOW MAPS *********
	const bool PreviousState = bUseOcclusionCulling;
	// Currently OCCLUSION_CULLING is not supported in shadow maps pass.
	bUseOcclusionCulling = false;

	CSM0 = nullptr;
	CSM1 = nullptr;
	CSM2 = nullptr;
	CSM3 = nullptr;

	FEShader* ShaderPBR = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	FEShader* ShaderInstancedPBR = RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
	FEShader* ShaderTerrain = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);

	// group<Component_TYPE> group takes ownership of the Component_TYPE.
	entt::basic_group GameModelGroup = CurrentScene->Registry.group<FEGameModelComponent>(entt::get<FETransformComponent>);
	entt::basic_view TerrainView = CurrentScene->Registry.view<FETerrainComponent, FETransformComponent>();

	for (std::string EntityID: LightsIDList)
	{
		FEEntity* LightEntity = CurrentScene->GetEntity(EntityID);
		FETransformComponent& TransformComponent = LightEntity->GetComponent<FETransformComponent>();
		FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

		if (LightComponent.GetType() != FE_DIRECTIONAL_LIGHT)
			continue;

		if (LightComponent.IsCastShadows())
		{
			const float ShadowsBlurFactor = LightComponent.GetShadowBlurFactor();
			ShaderPBR->UpdateParameterData("shadowBlurFactor", ShadowsBlurFactor);
			ShaderInstancedPBR->UpdateParameterData("shadowBlurFactor", ShadowsBlurFactor);

			const glm::vec3 OldCameraPosition = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE);
			const glm::mat4 OldViewMatrix = CurrentCameraComponent.GetViewMatrix();
			const glm::mat4 OldProjectionMatrix = CurrentCameraComponent.GetProjectionMatrix();

			for (size_t i = 0; i < static_cast<size_t>(LightComponent.ActiveCascades); i++)
			{
				// Put camera to the position of light.
				CurrentCameraComponent.ProjectionMatrix = LightComponent.CascadeData[i].ProjectionMat;
				CurrentCameraComponent.ViewMatrix = LightComponent.CascadeData[i].ViewMat;

				FE_GL_ERROR(glViewport(0, 0, LightComponent.CascadeData[i].FrameBuffer->GetWidth(), LightComponent.CascadeData[i].FrameBuffer->GetHeight()));

				UpdateGPUCullingFrustum();

				LightComponent.CascadeData[i].FrameBuffer->Bind();
				FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));

				for (auto [EnTTEntity, TerrainComponent, TransformComponent] : TerrainView.each())
				{
					FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
					if (Entity == nullptr)
						continue;

					if (!TerrainComponent.IsCastingShadows() || !TerrainComponent.IsVisible())
						continue;

					TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
					RenderTerrainComponent(Entity);
					TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
				}

				// FIX ME! No prefab support.
				for (entt::entity EnTTEntity : GameModelGroup)
				{
					auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

					if (!GameModelComponent.IsCastShadows() || !GameModelComponent.IsVisible())
						continue;

					FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
					FEMaterial* OriginalMaterial = GameModelComponent.GameModel->Material;
					
					FEMaterial* ShadowMapMaterialToUse = !Entity->HasComponent<FEInstancedComponent>() ? ShadowMapMaterial : ShadowMapMaterialInstanced;
					GameModelComponent.GameModel->Material = ShadowMapMaterialToUse;
					ShadowMapMaterialToUse->SetAlbedoMap(OriginalMaterial->GetAlbedoMap());

					if (OriginalMaterial->GetAlbedoMap(1) != nullptr)
					{
						ShadowMapMaterialToUse->SetAlbedoMap(OriginalMaterial->GetAlbedoMap(1), 1);
						ShadowMapMaterialToUse->GetAlbedoMap(1)->Bind(1);
					}

					if (!Entity->HasComponent<FEInstancedComponent>())
					{
						RenderGameModelComponent(Entity, nullptr, false);
					}
					else if (Entity->HasComponent<FEInstancedComponent>())
					{
						RenderGameModelComponentWithInstanced(Entity, LightComponent.CascadeData[i].Frustum, nullptr, true, false);
					}

					GameModelComponent.GameModel->Material = OriginalMaterial;
					for (size_t k = 0; k < ShadowMapMaterial->Textures.size(); k++)
					{
						ShadowMapMaterialToUse->Textures[k] = nullptr;
						ShadowMapMaterialToUse->TextureBindings[k] = -1;
					}
				}

				LightComponent.CascadeData[i].FrameBuffer->UnBind();
				switch (i)
				{
					case 0: CSM0 = LightComponent.CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					case 1: CSM1 = LightComponent.CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					case 2: CSM2 = LightComponent.CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					case 3: CSM3 = LightComponent.CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					default:
						break;
				}
			}

			CurrentCameraTransformComponent.SetPosition(OldCameraPosition, FE_WORLD_SPACE);
			CurrentCameraComponent.ViewMatrix = OldViewMatrix;
			CurrentCameraComponent.ProjectionMatrix = OldProjectionMatrix;

			FE_GL_ERROR(glViewport(0, 0, SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight()));
			//FE_GL_ERROR(glViewport(0, 0, CurrentCameraComponent.GetRenderTargetWidth(), CurrentCameraComponent.GetRenderTargetHeight()));
			break;
		}
	}

	bUseOcclusionCulling = PreviousState;
	// ********* GENERATE SHADOW MAPS END *********
	
	// in current version only shadows from one directional light is supported.
	if (!OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].empty())
	{
		if (CSM0) CSM0->Bind(FE_CSM_UNIT);
		if (CSM1) CSM1->Bind(FE_CSM_UNIT + 1);
		if (CSM2) CSM2->Bind(FE_CSM_UNIT + 2);
		if (CSM3) CSM3->Bind(FE_CSM_UNIT + 3);
	}

	// ********* RENDER SCENE *********

	GBuffer->GFrameBuffer->Bind();

	const unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, attachments);

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	UpdateGPUCullingFrustum();

	// FIX ME! No prefab support.
	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		if (!GameModelComponent.IsVisible() || !GameModelComponent.IsPostprocessApplied())
			continue;

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			ForceShader(RESOURCE_MANAGER.GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
			// FIX ME! Camera should inffered from the scene.
			RenderGameModelComponent(Entity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{

			ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
			// FIX ME! Camera should inffered from the scene.
			RenderGameModelComponentWithInstanced(Entity, CurrentCamera->Frustum);
		}
	}

	// It is not renderer work to update interaction ray.
	// It should be done in the input update.
	auto VirtualUIIterator = CurrentScene->VirtualUIContextMap.begin();
	while (VirtualUIIterator != CurrentScene->VirtualUIContextMap.end())
	{
		auto VirtualUIContext = VirtualUIIterator->second;
		if (VirtualUIContext->bMouseMovePassThrough)
			VirtualUIContext->UpdateInteractionRay(CurrentCamera->GetPosition(), RENDERER.MouseRay);

		VirtualUIIterator++;
	}

	for (auto [EnTTEntity, TerrainComponent, TransformComponent] : TerrainView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!TerrainComponent.IsVisible())
			continue;

		RenderTerrainComponent(Entity);
	}

	GBuffer->GFrameBuffer->UnBind();
	ForceShader(nullptr);

	GBuffer->Albedo->Bind(0);
	GBuffer->Normals->Bind(1);
	GBuffer->MaterialProperties->Bind(2);
	GBuffer->Positions->Bind(3);
	GBuffer->ShaderProperties->Bind(4);
	
	// ************************************ SSAO ************************************
	UpdateSSAO();
	// ************************************ SSAO END ************************************

	// ************************************ COPYING DEPTH BUFFER ************************************
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GBuffer->GFrameBuffer->FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, SceneToTextureFB->FBO); // write to default framebuffer
	glBlitFramebuffer(0, 0, SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight(),
					  0, 0, SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight(),
					  GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	// ************************************ COPYING DEPTH BUFFER END ************************************

	GBuffer->Albedo->Bind(0);
	GBuffer->Normals->Bind(1);
	GBuffer->MaterialProperties->Bind(2);
	GBuffer->Positions->Bind(3);
	GBuffer->ShaderProperties->Bind(4);
	SSAO->FB->GetColorAttachment()->Bind(5);
	SceneToTextureFB->Bind();

	const unsigned int attachments_[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments_);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* FinalSceneShader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	FinalSceneShader->Start();
	FinalSceneShader->UpdateParameterData("SSAOActive", SSAO->bActive ? 1.0f : 0.0f);
	LoadStandardParams(FinalSceneShader, true);
	FinalSceneShader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	GBuffer->Albedo->UnBind();
	GBuffer->Normals->UnBind();
	GBuffer->MaterialProperties->UnBind();
	GBuffer->Positions->UnBind();
	GBuffer->ShaderProperties->UnBind();

	FinalSceneShader->Stop();

	glDepthMask(GL_TRUE);
	// Could impact depth pyramid construction( min vs max ).
	glDepthFunc(GL_LESS);

	// ********* RENDER INSTANCED LINE *********
	//FE_GL_ERROR(glDisable(GL_CULL_FACE));

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedLineBuffer));
	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, FE_MAX_LINES * sizeof(FELine), this->LinesBuffer.data()));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	InstancedLineShader->Start();
	InstancedLineShader->UpdateParameterData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	InstancedLineShader->UpdateParameterData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	InstancedLineShader->UpdateParameterData("resolution", glm::vec2(SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight()));
	InstancedLineShader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(InstancedLineVAO));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glEnableVertexAttribArray(1));
	FE_GL_ERROR(glEnableVertexAttribArray(2));
	FE_GL_ERROR(glEnableVertexAttribArray(3));
	FE_GL_ERROR(glEnableVertexAttribArray(4));
	FE_GL_ERROR(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, LineCounter));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glDisableVertexAttribArray(1));
	FE_GL_ERROR(glDisableVertexAttribArray(2));
	FE_GL_ERROR(glDisableVertexAttribArray(3));
	FE_GL_ERROR(glDisableVertexAttribArray(4));
	FE_GL_ERROR(glBindVertexArray(0));
	InstancedLineShader->Stop();

	/*FE_GL_ERROR(glEnable(GL_CULL_FACE));
	FE_GL_ERROR(glCullFace(GL_BACK));*/
	// ********* RENDER INSTANCED LINE END *********

	// ********* RENDER SKY *********
	entt::basic_view SkyDomeView = CurrentScene->Registry.view<FESkyDomeComponent, FETransformComponent>();
	for (auto [EnTTEntity, SkyDomeComponent, TransformComponent] : SkyDomeView.each())
	{
		FEEntity* CurrentEntity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (CurrentEntity == nullptr)
			continue;

		if (!SKY_DOME_SYSTEM.IsEnabled())
		{
			CurrentEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
			break;
		}

		CurrentEntity->GetComponent<FEGameModelComponent>().SetVisibility(true);
		RenderGameModelComponent(CurrentEntity);
		CurrentEntity->GetComponent<FEGameModelComponent>().SetVisibility(false);
		// Only one sky dome is supported.
		break;
	}
	// ********* RENDER SCENE END *********
	SceneToTextureFB->UnBind();

	//Generate the mipmaps of colorAttachment
	SceneToTextureFB->GetColorAttachment()->Bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// ********* POST_PROCESS EFFECTS *********
	// Because we render post process effects with screen quad
	// we will turn off write to depth buffer in order to get clear DB to be able to render additional objects
	glDepthMask(GL_FALSE);
	FinalScene = SceneToTextureFB->GetColorAttachment();
	FETexture* PrevStageTex = SceneToTextureFB->GetColorAttachment();

	for (size_t i = 0; i < PostProcessEffects.size(); i++)
	{
		FEPostProcess& Effect = *PostProcessEffects[i];
		for (size_t j = 0; j < Effect.Stages.size(); j++)
		{
			Effect.Stages[j]->Shader->Start();
			LoadStandardParams(Effect.Stages[j]->Shader, nullptr, nullptr);
			for (size_t k = 0; k < Effect.Stages[j]->StageSpecificUniforms.size(); k++)
			{
				FEShaderParam* Param = Effect.Stages[j]->Shader->GetParameter(Effect.Stages[j]->StageSpecificUniforms[k].GetName());
				if (Param != nullptr)
				{
					Param->Data = Effect.Stages[j]->StageSpecificUniforms[k].Data;
				}
			}

			Effect.Stages[j]->Shader->LoadDataToGPU();

			for (size_t k = 0; k < Effect.Stages[j]->InTextureSource.size(); k++)
			{
				if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0)
				{
					Effect.Stages[j]->InTexture[k] = PrevStageTex;
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
				else if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_SCENE_HDR_COLOR)
				{
					Effect.Stages[j]->InTexture[k] = SceneToTextureFB->GetColorAttachment();
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
				else if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_SCENE_DEPTH)
				{
					Effect.Stages[j]->InTexture[k] = SceneToTextureFB->GetDepthAttachment();
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
				else if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_OWN_TEXTURE)
				{
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
			}

			FETexture* OrdinaryColorAttachment = Effect.IntermediateFramebuffer->GetColorAttachment();
			Effect.IntermediateFramebuffer->SetColorAttachment(Effect.Stages[j]->OutTexture);
			if (Effect.Stages[j]->OutTexture->Width != SceneToTextureFB->GetWidth())
			{
				FE_GL_ERROR(glViewport(0, 0, Effect.Stages[j]->OutTexture->Width, Effect.Stages[j]->OutTexture->Height));
			}
			else
			{
				FE_GL_ERROR(glViewport(0, 0, SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight()));
				//FE_GL_ERROR(glViewport(0, 0, CurrentCameraComponent.GetRenderTargetWidth(), CurrentCameraComponent.GetRenderTargetHeight()));
			}
			Effect.IntermediateFramebuffer->Bind();

			FE_GL_ERROR(glBindVertexArray(Effect.ScreenQuad->GetVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, Effect.ScreenQuad->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			Effect.IntermediateFramebuffer->UnBind();
			// this was added because of postProcesses and how they "manage" colorAttachment of FB
			Effect.IntermediateFramebuffer->SetColorAttachment(OrdinaryColorAttachment);

			for (size_t k = 0; k < Effect.Stages[j]->InTextureSource.size(); k++)
			{
				Effect.Stages[j]->InTexture[k]->UnBind();
			}
			Effect.Stages[j]->Shader->Stop();

			PrevStageTex = Effect.Stages[j]->OutTexture;
		}
	}

	for (int i = static_cast<int>(PostProcessEffects.size() - 1); i >= 0; i--)
	{
		FEPostProcess& Effect = *PostProcessEffects[i];
		
		if (Effect.bActive)
		{
			Effect.RenderResult();
			FinalScene = Effect.Stages.back()->OutTexture;
			break;
		}
	}

	glDepthMask(GL_TRUE);
	// ********* SCREEN SPACE EFFECTS END *********

	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI *********
	FETexture* OriginalColorAttachment = SceneToTextureFB->GetColorAttachment();
	SceneToTextureFB->SetColorAttachment(FinalScene);

	SceneToTextureFB->Bind();

	// FIX ME! No prefab support.
	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		if (!GameModelComponent.IsVisible() || GameModelComponent.IsPostprocessApplied())
			continue;

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			RenderGameModelComponent(Entity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			RenderGameModelComponentWithInstanced(Entity, CurrentCamera->Frustum);
		}
	}
	
	SceneToTextureFB->UnBind();
	SceneToTextureFB->SetColorAttachment(OriginalColorAttachment);
	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI END *********

	// **************************** TERRAIN EDITOR TOOLS ****************************
	TERRAIN_SYSTEM.UpdateBrush(CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay);
	// **************************** TERRAIN EDITOR TOOLS END ****************************

	LineCounter = 0;

	// **************************** DEPTH PYRAMID ****************************
#ifdef USE_OCCLUSION_CULLING

	ComputeTextureCopy->Start();
	ComputeTextureCopy->UpdateParameterData("textureSize", glm::vec2(RENDERER.DepthPyramid->GetWidth(), RENDERER.DepthPyramid->GetHeight()));
	ComputeTextureCopy->LoadDataToGPU();

	RENDERER.SceneToTextureFB->GetDepthAttachment()->Bind(0);
	glBindImageTexture(1, RENDERER.DepthPyramid->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	ComputeTextureCopy->Dispatch(static_cast<unsigned>(ceil(float(RENDERER.DepthPyramid->GetWidth()) / 32.0f)), static_cast<unsigned>(ceil(float(RENDERER.DepthPyramid->GetHeight()) / 32.0f)), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const size_t MipCount = static_cast<size_t>(floor(log2(std::max(RENDERER.DepthPyramid->GetWidth(), RENDERER.DepthPyramid->GetHeight()))) + 1);
	for (size_t i = 0; i < MipCount; i++)
	{
		const float DownScale = static_cast<float>(pow(2.0f, i));

		ComputeDepthPyramidDownSample->Start();
		ComputeDepthPyramidDownSample->UpdateParameterData("textureSize", glm::vec2(RENDERER.DepthPyramid->GetWidth() / DownScale, RENDERER.DepthPyramid->GetHeight() / DownScale));
		ComputeDepthPyramidDownSample->LoadDataToGPU();
		glBindImageTexture(0, RENDERER.DepthPyramid->GetTextureID(), static_cast<GLint>(i), GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(1, RENDERER.DepthPyramid->GetTextureID(), static_cast<GLint>(i + 1), GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		ComputeDepthPyramidDownSample->Dispatch(static_cast<unsigned>(ceil(float(RENDERER.DepthPyramid->GetWidth() / DownScale) / 32.0f)), static_cast<unsigned>(ceil(float(RENDERER.DepthPyramid->GetHeight() / DownScale) / 32.0f)), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
#endif // USE_OCCLUSION_CULLING
	// **************************** DEPTH PYRAMID END ****************************
}

FEPostProcess* FERenderer::GetPostProcessEffect(const std::string ID)
{
	for (size_t i = 0; i < PostProcessEffects.size(); i++)
	{
		if (PostProcessEffects[i]->GetObjectID() == ID)
			return PostProcessEffects[i];
	}

	return nullptr;
}

std::vector<std::string> FERenderer::GetPostProcessList()
{
	std::vector<std::string> Result;
	for (size_t i = 0; i < PostProcessEffects.size(); i++)
		Result.push_back(PostProcessEffects[i]->GetObjectID());
		
	return Result;
}

void FERenderer::TakeScreenshot(const char* FileName, const int Width, const int Height)
{
	unsigned char* pixels = new unsigned char[4 * Width * Height];
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, PostProcessEffects.back()->Stages.back()->OutTexture->GetTextureID()));
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	
	FETexture* TempTexture = RESOURCE_MANAGER.RawDataToFETexture(pixels, Width, Height);
	RESOURCE_MANAGER.SaveFETexture(TempTexture, FileName);
	RESOURCE_MANAGER.DeleteFETexture(TempTexture);

	delete[] pixels;
}

void FERenderer::RenderGameModelComponent(FEEntity* Entity, FEEntity* ForceCamera, bool bReloadUniformBlocks)
{
	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();

	if (GameModelComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (bReloadUniformBlocks)
		LoadUniformBlocks(Entity->ParentScene);

	FEGameModel* GameModel = GameModelComponent.GameModel;
	if (GameModel == nullptr)
	{
		LOG.Add("Trying to draw FEGameModelComponent with GameModel that is nullptr in FERenderer::RenderGameModelComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	FEShader* OriginalShader = GameModel->Material->Shader;
	if (ShaderToForce)
	{
		if (OriginalShader->GetName() == "FEPBRShader")
			GameModel->Material->Shader = ShaderToForce;
	}

	GameModel->Material->Bind();
	LoadStandardParams(GameModel->Material->Shader, GameModel->Material, &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	GameModel->Material->Shader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(GameModel->Mesh->GetVaoID()));
	if ((GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
	if ((GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

	if ((GameModel->Mesh->VertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
	if ((GameModel->Mesh->VertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, GameModel->Mesh->GetVertexCount()));

	FE_GL_ERROR(glBindVertexArray(0));

	GameModel->Material->UnBind();

	if (ShaderToForce)
	{
		if (OriginalShader->GetName() == "FEPBRShader")
			GameModel->Material->Shader = OriginalShader;
	}

	if (GameModelComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FERenderer::RenderGameModelComponentForward(FEEntity* Entity, FEEntity* ForceCamera, bool bReloadUniformBlocks)
{
	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();

	if (bReloadUniformBlocks)
		LoadUniformBlocks(Entity->ParentScene);

	FEGameModel* GameModel = GameModelComponent.GameModel;
	if (GameModel == nullptr)
	{
		LOG.Add("Trying to draw FEGameModelComponent with GameModel that is nullptr in FERenderer::RenderGameModelComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	FEShader* OriginalShader = nullptr;
	if (!bSimplifiedRendering || RENDERER.bVRActive)
	{
		OriginalShader = GameModel->Material->Shader;
		if (RENDERER.bVRActive)
		{
			if (OriginalShader->GetObjectID() != "6917497A5E0C05454876186F"/*"SolidColorMaterial"*/)
				GameModel->Material->Shader = RESOURCE_MANAGER.GetShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
		}
		else
		{
			GameModel->Material->Shader = RESOURCE_MANAGER.GetShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
		}
	}

	GameModel->Material->Bind();
	LoadStandardParams(GameModel->Material->Shader, GameModel->Material, &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	GameModel->Material->Shader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(GameModel->Mesh->GetVaoID()));
	if ((GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
	if ((GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

	if ((GameModel->Mesh->VertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
	if ((GameModel->Mesh->VertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, GameModel->Mesh->GetVertexCount()));

	FE_GL_ERROR(glBindVertexArray(0));

	GameModel->Material->UnBind();

	if (!bSimplifiedRendering || RENDERER.bVRActive)
		GameModel->Material->Shader = OriginalShader;
}

void FERenderer::RenderTerrainComponent(FEEntity* TerrainEntity, FEEntity* ForceCamera)
{
	if (TerrainEntity == nullptr)
	{
		LOG.Add("FERenderer::RenderTerrainComponent TerrainEntity is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (!TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FERenderer::RenderTerrainComponent TerrainEntity does not have valid FETerrainComponent", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	FETransformComponent& TransformComponent = TerrainEntity->GetComponent<FETransformComponent>();
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();
	
	if (TerrainComponent.Shader == nullptr)
	{
		LOG.Add("FERenderer::RenderTerrainComponent TerrainComponent does not have valid Shader", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.HeightMap == nullptr)
	{
		LOG.Add("FERenderer::RenderTerrainComponent TerrainComponent does not have valid HeightMap", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (TerrainComponent.Shader->GetName() == "FESMTerrainShader")
	{
		TerrainComponent.HeightMap->Bind(0);
	}
	else
	{
		for (size_t i = 0; i < TerrainComponent.Layers.size(); i++)
		{
			if (TerrainComponent.Layers[i] != nullptr && TerrainComponent.Layers[i]->GetMaterial()->IsCompackPacking())
			{
				if (TerrainComponent.Layers[i]->GetMaterial()->GetAlbedoMap() != nullptr)
					TerrainComponent.Layers[i]->GetMaterial()->GetAlbedoMap()->Bind(static_cast<int>(i * 3));

				if (TerrainComponent.Layers[i]->GetMaterial()->GetNormalMap() != nullptr)
					TerrainComponent.Layers[i]->GetMaterial()->GetNormalMap()->Bind(static_cast<int>(i * 3 + 1));

				if (TerrainComponent.Layers[i]->GetMaterial()->GetAOMap() != nullptr)
					TerrainComponent.Layers[i]->GetMaterial()->GetAOMap()->Bind(static_cast<int>(i * 3 + 2));
			}
		}

		TerrainComponent.HeightMap->Bind(24);
		if (TerrainComponent.ProjectedMap != nullptr)
			TerrainComponent.ProjectedMap->Bind(25);

		for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE; i++)
		{
			if (TerrainComponent.LayerMaps[i] != nullptr)
				TerrainComponent.LayerMaps[i]->Bind(static_cast<int>(26 + i));
		}
	}

	TerrainComponent.Shader->Start();
	LoadStandardParams(TerrainComponent.Shader, nullptr, &TransformComponent, ForceCamera, TerrainComponent.IsReceivingShadows());
	// ************ Load materials data for all Terrain layers ************

	const int LayersUsed = TerrainComponent.LayersUsed();
	if (LayersUsed == 0)
	{
		// 0 index is for hightMap.
		RESOURCE_MANAGER.NoTexture->Bind(1);
	}

	TerrainComponent.LoadLayersDataToGPU();
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, TerrainComponent.GPULayersDataBuffer));

	// Shadow map shader does not have this parameter.
	if (TerrainComponent.Shader->GetParameter("usedLayersCount") != nullptr)
		TerrainComponent.Shader->UpdateParameterData("usedLayersCount", static_cast<float>(LayersUsed));
	// ************ Load materials data for all Terrain layers END ************

	TerrainComponent.Shader->UpdateParameterData("hightScale", TerrainComponent.HightScale);
	TerrainComponent.Shader->UpdateParameterData("scaleFactor", TerrainComponent.ScaleFactor);
	if (TerrainComponent.Shader->GetName() != "FESMTerrainShader")
		TerrainComponent.Shader->UpdateParameterData("tileMult", TerrainComponent.TileMult);
	TerrainComponent.Shader->UpdateParameterData("LODlevel", TerrainComponent.LODLevel);
	TerrainComponent.Shader->UpdateParameterData("hightMapShift", TerrainComponent.HightMapShift);

	glm::vec3 PivotPosition = TransformComponent.GetPosition();
	TerrainComponent.ScaleFactor = 1.0f * TerrainComponent.ChunkPerSide;

	static int PVMHash = static_cast<int>(std::hash<std::string>{}("FEPVMMatrix"));
	static int WorldMatrixHash = static_cast<int>(std::hash<std::string>{}("FEWorldMatrix"));
	static int HightMapShiftHash = static_cast<int>(std::hash<std::string>{}("hightMapShift"));

	TerrainComponent.Shader->LoadDataToGPU();
	FETransformComponent OldState = TransformComponent;
	for (size_t i = 0; i < TerrainComponent.ChunkPerSide; i++)
	{
		for (size_t j = 0; j < TerrainComponent.ChunkPerSide; j++)
		{
			// Kind of hacky code
			// Revert to old state to avoid any changes in TransformComponent.
			TransformComponent = OldState;
			glm::mat4 ParentMatrix = TransformComponent.GetParentMatrix();
			// Use here SetWorldPosition instead to avoid usage of LocalSpaceMatrix directly.
			TransformComponent.SetPosition(glm::vec3(PivotPosition.x + i * 64.0f * TransformComponent.Scale[0], PivotPosition.y, PivotPosition.z + j * 64.0f * TransformComponent.Scale[2]));
			// Not to wait for scene hierarchy update.
			TransformComponent.WorldSpaceMatrix = ParentMatrix * TransformComponent.GetLocalMatrix();

			//  FIX ME!
			FEEntity* CameraEntityToUse = ForceCamera;
			if (ForceCamera == nullptr)
			{
				CameraEntityToUse = TryToGetLastUsedCameraEntity();
				if (CameraEntityToUse == nullptr)
					return;
			}
			//  FIX ME!
			FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();
			FETransformComponent& CurrentCameraTransformComponent = CameraEntityToUse->GetComponent<FETransformComponent>();

			TerrainComponent.Shader->UpdateParameterData("FEPVMMatrix", CurrentCameraComponent.GetProjectionMatrix() * CurrentCameraComponent.GetViewMatrix() * TransformComponent.GetWorldMatrix());
			if (TerrainComponent.Shader->GetParameter("FEWorldMatrix") != nullptr)
				TerrainComponent.Shader->UpdateParameterData("FEWorldMatrix", TransformComponent.GetWorldMatrix());
			TerrainComponent.Shader->UpdateParameterData("hightMapShift", glm::vec2(i * -1.0f, j * -1.0f));

			TerrainComponent.Shader->LoadMatrix(PVMHash, *static_cast<glm::mat4*>(TerrainComponent.Shader->GetParameter("FEPVMMatrix")->Data));
			if (TerrainComponent.Shader->GetParameter("FEWorldMatrix") != nullptr)
				TerrainComponent.Shader->LoadMatrix(WorldMatrixHash, *static_cast<glm::mat4*>(TerrainComponent.Shader->GetParameter("FEWorldMatrix")->Data));

			if (TerrainComponent.Shader->GetParameter("hightMapShift") != nullptr)
				TerrainComponent.Shader->LoadVector(HightMapShiftHash, *static_cast<glm::vec2*>(TerrainComponent.Shader->GetParameter("hightMapShift")->Data));

			FE_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64));
		}
	}
	TerrainComponent.Shader->Stop();
	// Revert to old state to avoid any changes in TransformComponent.
	TransformComponent = OldState;

	if (TerrainComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FERenderer::DrawLine(const glm::vec3 BeginPoint, const glm::vec3 EndPoint, const glm::vec3 Color, const float Width)
{
	if (LineCounter >= FE_MAX_LINES)
	{
		LOG.Add("Tring to draw more than maxLines", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	LinesBuffer[LineCounter].Begin = BeginPoint;
	LinesBuffer[LineCounter].End = EndPoint;
	LinesBuffer[LineCounter].Color = Color;
	LinesBuffer[LineCounter].Width = Width;

	LineCounter++;
}

bool FERenderer::IsDistanceFogEnabled()
{
	return bDistanceFogEnabled;
}

void FERenderer::SetDistanceFogEnabled(const bool NewValue)
{
	if (bDistanceFogEnabled == false && NewValue == true)
	{
		if (DistanceFogDensity <= 0.0f)
			DistanceFogDensity = 0.007f;
		if (DistanceFogGradient <= 0.0f)
			DistanceFogGradient = 2.5f;
	}
	bDistanceFogEnabled = NewValue;
	UpdateFogInShaders();
}

float FERenderer::GetDistanceFogDensity()
{
	return DistanceFogDensity;
}

void FERenderer::SetDistanceFogDensity(const float NewValue)
{
	DistanceFogDensity = NewValue;
	UpdateFogInShaders();
}

float FERenderer::GetDistanceFogGradient()
{
	return DistanceFogGradient;
}

void FERenderer::SetDistanceFogGradient(const float NewValue)
{
	DistanceFogGradient = NewValue;
	UpdateFogInShaders();
}

void FERenderer::UpdateFogInShaders()
{
	if (bDistanceFogEnabled)
	{
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("fogDensity", DistanceFogDensity);
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("fogGradient", DistanceFogGradient);

		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("fogDensity", DistanceFogDensity);
		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("fogGradient", DistanceFogGradient);
	}
	else
	{
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("fogDensity", -1.0f);
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateParameterData("fogGradient", -1.0f);

		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("fogDensity", -1.0f);
		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateParameterData("fogGradient", -1.0f);
	}
}

float FERenderer::GetChromaticAberrationIntensity()
{
	if (GetPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/)->Stages[0]->Shader->
		GetParameter("intensity")->Data);
}

void FERenderer::SetChromaticAberrationIntensity(const float NewValue)
{
	if (GetPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/) == nullptr)
		return;
	GetPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/)->Stages[0]->Shader->UpdateParameterData("intensity", NewValue);
}

float FERenderer::GetDOFNearDistance()
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->
		GetParameter("depthThreshold")->Data);
}

void FERenderer::SetDOFNearDistance(const float NewValue)
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->UpdateParameterData("depthThreshold", NewValue);
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[1]->Shader->UpdateParameterData("depthThreshold", NewValue);
}

float FERenderer::GetDOFFarDistance()
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->
		GetParameter("depthThresholdFar")->Data);
}

void FERenderer::SetDOFFarDistance(const float NewValue)
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->UpdateParameterData("depthThresholdFar", NewValue);
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[1]->Shader->UpdateParameterData("depthThresholdFar", NewValue);
}

float FERenderer::GetDOFStrength()
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->
		GetParameter("blurSize")->Data);
}

void FERenderer::SetDOFStrength(const float NewValue)
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->UpdateParameterData("blurSize", NewValue);
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[1]->Shader->UpdateParameterData("blurSize", NewValue);
}

float FERenderer::GetDOFDistanceDependentStrength()
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->
		GetParameter("intMult")->Data);
}

void FERenderer::SetDOFDistanceDependentStrength(const float NewValue)
{
	if (GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[0]->Shader->UpdateParameterData("intMult", NewValue);
	GetPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->Stages[1]->Shader->UpdateParameterData("intMult", NewValue);
}

float FERenderer::GetBloomThreshold()
{
	if (GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->Stages[0]->Shader->GetParameter(
		"thresholdBrightness")->Data);
}

void FERenderer::SetBloomThreshold(const float NewValue)
{
	if (GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return;
	GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->Stages[0]->Shader->UpdateParameterData("thresholdBrightness", NewValue);
}

float FERenderer::GetBloomSize()
{
	if (GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->Stages[1]->StageSpecificUniforms[1].
		Data);
}

void FERenderer::SetBloomSize(const float NewValue)
{
	if (GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return;
	*static_cast<float*>(GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->Stages[1]->StageSpecificUniforms[1].Data) = NewValue;
	*static_cast<float*>(GetPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->Stages[2]->StageSpecificUniforms[1].Data) = NewValue;
}

float FERenderer::GetFXAASpanMax()
{
	if (GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->Stages[0]->Shader->
		GetParameter("FXAASpanMax")->Data);
}

void FERenderer::SetFXAASpanMax(const float NewValue)
{
	if (GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return;
	GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->Stages[0]->Shader->UpdateParameterData("FXAASpanMax", NewValue);
}

float FERenderer::GetFXAAReduceMin()
{
	if (GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->Stages[0]->Shader->
		GetParameter("FXAAReduceMin")->Data);
}

void FERenderer::SetFXAAReduceMin(const float NewValue)
{
	if (GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return;
	GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->Stages[0]->Shader->UpdateParameterData("FXAAReduceMin", NewValue);
}

float FERenderer::GetFXAAReduceMul()
{
	if (GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return 0.0f;
	return *static_cast<float*>(GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->Stages[0]->Shader->
		GetParameter("FXAAReduceMul")->Data);
}

void FERenderer::SetFXAAReduceMul(const float NewValue)
{
	if (GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return;
	GetPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->Stages[0]->Shader->UpdateParameterData("FXAAReduceMul", NewValue);
}

void FERenderer::DrawAABB(FEAABB AABB, const glm::vec3 Color, const float LineWidth)
{
	// bottom plane
	DrawLine(glm::vec3(AABB.GetMin()), glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMin()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMin()), glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMax()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMax()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMax()[2]), Color, LineWidth);

	// upper plane
	DrawLine(glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMin()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color, LineWidth);

	// conect two planes
	DrawLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMin()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMax()[0], AABB.GetMin()[1], AABB.GetMax()[2]), glm::vec3(AABB.GetMax()[0], AABB.GetMax()[1], AABB.GetMax()[2]), Color, LineWidth);
	DrawLine(glm::vec3(AABB.GetMin()[0], AABB.GetMin()[1], AABB.GetMin()[2]), glm::vec3(AABB.GetMin()[0], AABB.GetMax()[1], AABB.GetMin()[2]), Color, LineWidth);
}

void FERenderer::ForceShader(FEShader* Shader)
{
	ShaderToForce = Shader;
}

void FERenderer::UpdateGPUCullingFrustum()
{
	float* FrustumBufferData = static_cast<float*>(glMapNamedBufferRange(FrustumInfoBuffer, 0, sizeof(float) * (32),
	                                                                     GL_MAP_WRITE_BIT |
	                                                                     GL_MAP_INVALIDATE_BUFFER_BIT |
	                                                                     GL_MAP_UNSYNCHRONIZED_BIT));

	//  FIX ME!
	FEEntity* CameraEntityToUse = TryToGetLastUsedCameraEntity();
	if (CameraEntityToUse == nullptr)
		return;
	
	FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = CameraEntityToUse->GetComponent<FETransformComponent>();
	std::vector<std::vector<float>> CurrentFrustum = CurrentCameraComponent.GetFrustumPlanes();

	for (size_t i = 0; i < 6; i++)
	{
		FrustumBufferData[i * 4] = CurrentFrustum[i][0];
		FrustumBufferData[i * 4 + 1] = CurrentFrustum[i][1];
		FrustumBufferData[i * 4 + 2] = CurrentFrustum[i][2];
		FrustumBufferData[i * 4 + 3] = CurrentFrustum[i][3];
	}

	FrustumBufferData[24] = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE)[0];
	FrustumBufferData[25] = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE)[1];
	FrustumBufferData[26] = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE)[2];

	FE_GL_ERROR(glUnmapNamedBuffer(FrustumInfoBuffer));
}

void FERenderer::GPUCulling(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	if (bFreezeCulling)
		return;

	INSTANCED_RENDERING_SYSTEM.CheckDirtyFlag(TransformComponent, GameModelComponent, InstancedComponent);

	FrustumCullingShader->Start();

	//  FIX ME!
	FEEntity* CameraEntityToUse = TryToGetLastUsedCameraEntity();
	if (CameraEntityToUse == nullptr)
		return;
	FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = CameraEntityToUse->GetComponent<FETransformComponent>();

#ifdef USE_OCCLUSION_CULLING
	FrustumCullingShader->UpdateParameterData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	FrustumCullingShader->UpdateParameterData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	FrustumCullingShader->UpdateParameterData("useOcclusionCulling", bUseOcclusionCulling);
	// It should be last frame size!
	const glm::vec2 RenderTargetSize = glm::vec2(GBuffer->GFrameBuffer->DepthAttachment->GetWidth(), GBuffer->GFrameBuffer->DepthAttachment->GetHeight());
	FrustumCullingShader->UpdateParameterData("renderTargetSize", RenderTargetSize);
	FrustumCullingShader->UpdateParameterData("nearFarPlanes", glm::vec2(CurrentCameraComponent.GetNearPlane(), CurrentCameraComponent.GetFarPlane()));
#endif // USE_OCCLUSION_CULLING

	FrustumCullingShader->LoadDataToGPU();

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InstancedComponent.SourceDataBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, InstancedComponent.PositionsBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FrustumInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, InstancedComponent.LODBuffers[0]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, InstancedComponent.AABBSizesBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, CullingLODCountersBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, InstancedComponent.LODInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, InstancedComponent.LODBuffers[1]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, InstancedComponent.LODBuffers[2]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, InstancedComponent.LODBuffers[3]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, InstancedComponent.IndirectDrawInfoBuffer));

	DepthPyramid->Bind(0);
	FrustumCullingShader->Dispatch(static_cast<GLuint>(ceil(InstancedComponent.InstanceCount / 64.0f)), 1, 1);
	FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));
}

std::unordered_map<std::string, std::function<FETexture* ()>> FERenderer::GetDebugOutputTextures()
{
	return DebugOutputTextures;
}

void FERenderer::RenderTargetResize(const int NewWidth, const int NewHeight)
{
	delete SceneToTextureFB;
	SceneToTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, NewWidth, NewHeight);

	if (bVRActive)
	{
		if (VRScreenW != 0 && VRScreenH != 0)
		{
			delete SceneToVRTextureFB;
			SceneToVRTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, VRScreenW, VRScreenH);
		}
	}

	if (bSimplifiedRendering)
		return;

	delete DepthPyramid;
	DepthPyramid = RESOURCE_MANAGER.CreateTexture();
	RESOURCE_MANAGER.Textures.erase(DepthPyramid->GetObjectID());

	DepthPyramid->Bind();
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	const int MaxDimention = std::max(NewWidth, NewHeight);
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_R32F, NewWidth, NewHeight));
	DepthPyramid->Width = NewWidth;
	DepthPyramid->Height = NewHeight;

	GBuffer->RenderTargetResize(SceneToTextureFB);
	SSAO->RenderTargetResize(SceneToTextureFB);
	
	for (size_t i = 0; i < PostProcessEffects.size(); i++)
	{
		// We should delete only internally created frame buffers.
		// Other wise user created postProcess could create UB.
		if (PostProcessEffects[i]->GetName() == "bloom" || 
			PostProcessEffects[i]->GetName() == "GammaAndHDR" ||
			PostProcessEffects[i]->GetName() == "FE_FXAA" || 
			PostProcessEffects[i]->GetName() == "DOF" || 
			PostProcessEffects[i]->GetName() == "chromaticAberration")
		delete PostProcessEffects[i];
	}

	PostProcessEffects.clear();
}

bool FERenderer::IsOcclusionCullingEnabled()
{
	return bUseOcclusionCulling;
}

void FERenderer::SetOcclusionCullingEnabled(const bool NewValue)
{
	bUseOcclusionCulling = NewValue;
}

void FERenderer::UpdateSSAO()
{
	if (!SSAO->bActive)
		return;

	SSAO->FB->Bind();
	if (SSAO->Shader == nullptr)
		SSAO->Shader = RESOURCE_MANAGER.GetShader("1037115B676E383E36345079"/*"FESSAOShader"*/);

	SSAO->Shader->UpdateParameterData("SampleCount", SSAO->SampleCount);
	
	SSAO->Shader->UpdateParameterData("SmallDetails", SSAO->bSmallDetails ? 1.0f : 0.0f);
	SSAO->Shader->UpdateParameterData("Bias", SSAO->Bias);
	SSAO->Shader->UpdateParameterData("Radius", SSAO->Radius);
	SSAO->Shader->UpdateParameterData("RadiusSmallDetails", SSAO->RadiusSmallDetails);
	SSAO->Shader->UpdateParameterData("SmallDetailsWeight", SSAO->SmallDetailsWeight);
	
	SSAO->Shader->Start();
	LoadStandardParams(SSAO->Shader, true, nullptr);
	SSAO->Shader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	SSAO->Shader->Stop();

	if (SSAO->bBlured)
	{
		// First blur stage
		FEShader* BlurShader = RESOURCE_MANAGER.GetShader("0B5770660B6970800D776542"/*"FESSAOBlurShader"*/);
		BlurShader->Start();
		if (BlurShader->GetParameter("FEBlurDirection"))
			BlurShader->UpdateParameterData("FEBlurDirection", glm::vec2(0.0f, 1.0f));
		if (BlurShader->GetParameter("BlurRadius"))
			BlurShader->UpdateParameterData("BlurRadius", 1.3f);

		BlurShader->LoadDataToGPU();

		SSAO->FB->GetColorAttachment()->Bind(0);
		SceneToTextureFB->GetDepthAttachment()->Bind(1);
		GBuffer->Normals->Bind(2);
		FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		// Second blur stage
		if (BlurShader->GetParameter("FEBlurDirection"))
			BlurShader->UpdateParameterData("FEBlurDirection", glm::vec2(1.0f, 0.0f));
		if (BlurShader->GetParameter("BlurRadius"))
			BlurShader->UpdateParameterData("BlurRadius", 1.3f);

		BlurShader->LoadDataToGPU();

		SSAO->FB->GetColorAttachment()->Bind(0);
		SceneToTextureFB->GetDepthAttachment()->Bind(1);
		GBuffer->Normals->Bind(2);
		FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		BlurShader->Stop();

		SSAO->FB->GetColorAttachment()->UnBind();
		SSAO->FB->UnBind();
	}
}

bool FERenderer::IsSSAOEnabled()
{
	return SSAO->bActive;
}

void FERenderer::SetSSAOEnabled(const bool NewValue)
{
	SSAO->bActive = NewValue;
}

int FERenderer::GetSSAOSampleCount()
{
	return SSAO->SampleCount;
}

void FERenderer::SetSSAOSampleCount(int NewValue)
{
	if (NewValue < 1)
		NewValue = 1;

	if (NewValue > 64)
		NewValue = 64;

	SSAO->SampleCount = NewValue;
}

bool FERenderer::IsSSAOSmallDetailsEnabled()
{
	return SSAO->bSmallDetails;
}

void FERenderer::SetSSAOSmallDetailsEnabled(const bool NewValue)
{
	SSAO->bSmallDetails = NewValue;
}

bool FERenderer::IsSSAOResultBlured()
{
	return SSAO->bBlured;
}

void FERenderer::SetSSAOResultBlured(const bool NewValue)
{
	SSAO->bBlured = NewValue;
}

float FERenderer::GetSSAOBias()
{
	return SSAO->Bias;
}

void FERenderer::SetSSAOBias(const float NewValue)
{
	SSAO->Bias = NewValue;
}

float FERenderer::GetSSAORadius()
{
	return SSAO->Radius;
}

void FERenderer::SetSSAORadius(const float NewValue)
{
	SSAO->Radius = NewValue;
}

float FERenderer::GetSSAORadiusSmallDetails()
{
	return SSAO->RadiusSmallDetails;
}

void FERenderer::SetSSAORadiusSmallDetails(const float NewValue)
{
	SSAO->RadiusSmallDetails = NewValue;
}

float FERenderer::GetSSAOSmallDetailsWeight()
{
	return SSAO->SmallDetailsWeight;
}

void FERenderer::SetSSAOSmallDetailsWeight(const float NewValue)
{
	SSAO->SmallDetailsWeight = NewValue;
}

void FERenderer::InitVR(int VRScreenW, int VRScreenH)
{
	UpdateVRRenderTargetSize( VRScreenW, VRScreenH);
}

void FERenderer::UpdateVRRenderTargetSize(int VRScreenW, int VRScreenH)
{
	this->VRScreenW = VRScreenW;
	this->VRScreenH = VRScreenH;

	if (VRScreenW != 0 && VRScreenH != 0)
	{
		delete SceneToVRTextureFB;
		SceneToVRTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, VRScreenW, VRScreenH);
	}
}

void FERenderer::RenderVR(FEScene* CurrentScene, FEBasicCamera* CurrentCamera)
{
	if (CurrentScene == nullptr || CurrentCamera == nullptr || SceneToVRTextureFB == nullptr)
		return;

	// glViewPort and Frame buffer already are set.
	CurrentCamera->UpdateFrustumPlanes();

	SceneToVRTextureFB->Bind();
	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	LoadUniformBlocks(CurrentScene);

	// No instanced rendering for now.
	entt::basic_group GameModelGroup = CurrentScene->Registry.group<FEGameModelComponent>(entt::get<FETransformComponent>, entt::exclude<FEInstancedComponent>);
	for (entt::entity CurrentEnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(CurrentEnTTEntity);

		FEEntity* CurrentEntity = CurrentScene->GetEntityByEnTT(CurrentEnTTEntity);
		if (CurrentEntity == nullptr)
			continue;

		if (GameModelComponent.IsVisible() && GameModelComponent.IsPostprocessApplied())
		{
			// FIX ME! CAMERA
			RenderGameModelComponentForward(CurrentEntity);
		}
	}

	SceneToVRTextureFB->UnBind();
	LastRenderedResult = SceneToVRTextureFB;

	for (size_t i = 0; i < AfterRenderCallbacks.size(); i++)
	{
		AfterRenderCallbacks[i]();
	}
}

void FERenderer::RenderToFrameBuffer(FETexture* SceneTexture, FEFramebuffer* Target)
{
	if (SceneTexture == nullptr)
	{
		LOG.Add("Tring to call FERenderer::RenderToFrameBuffer with SceneTexture = nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (Target == nullptr)
	{
		RenderToFrameBuffer(SceneTexture, static_cast<GLuint>(0));
		return;
	}

	RenderToFrameBuffer(SceneTexture, Target->FBO);
}

void FERenderer::RenderToFrameBuffer(FETexture* SceneTexture, GLuint Target)
{
	SceneTexture->Bind(0);
	FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, Target));

	FE_GL_ERROR(glDepthMask(GL_FALSE));
	FE_GL_ERROR(glDepthFunc(GL_ALWAYS));

	FEShader* ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	ScreenQuadShader->Start();
	ScreenQuadShader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	ScreenQuadShader->Stop();

	FE_GL_ERROR(glDepthMask(GL_TRUE));
	FE_GL_ERROR(glDepthFunc(GL_LESS));

	SceneTexture->UnBind();
	if (Target != 0)
		FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

bool FERenderer::CombineFrameBuffers(FEFramebuffer* FirstSource, FEFramebuffer* SecondSource, FEFramebuffer* Target)
{
	if (FirstSource == nullptr)
	{
		LOG.Add("Attempted to call FERenderer::CombineFrameBuffers with FirstSource set to nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (SecondSource == nullptr)
	{
		LOG.Add("Attempted to call FERenderer::CombineFrameBuffers with SecondSource set to nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource->GetColorAttachment() == nullptr || SecondSource->GetColorAttachment() == nullptr)
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, either the FirstSource or SecondSource is missing a ColorAttachment.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource->GetDepthAttachment() == nullptr || SecondSource->GetDepthAttachment() == nullptr)
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, either the FirstSource or SecondSource is missing a DepthAttachment, which is currently unsupported.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource->GetColorAttachment(1) != nullptr || SecondSource->GetColorAttachment(1) != nullptr)
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, either the FirstSource or SecondSource have multiple ColorAttachments, which is currently unsupported.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource == Target || SecondSource == Target)
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, Sources and Target should be different framebuffers.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource->GetColorAttachment()->GetWidth() != SecondSource->GetColorAttachment()->GetWidth() ||
		FirstSource->GetColorAttachment()->GetHeight() != SecondSource->GetColorAttachment()->GetHeight())
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, FirstSource and SecondSource ColorAttachment have different sizes.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource->GetColorAttachment()->GetWidth() != Target->GetColorAttachment()->GetWidth() ||
		FirstSource->GetColorAttachment()->GetHeight() != Target->GetColorAttachment()->GetHeight())
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, Sources and Target ColorAttachment have different sizes.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	Target->Bind();

	// Although we are using screen quad texture, in this function we need to write to a depth buffer.
	glDepthMask(GL_TRUE);
	// Bypass depth buffer checks.
	glDepthFunc(GL_ALWAYS);

	FirstSource->GetColorAttachment()->Bind(0);
	FirstSource->GetDepthAttachment()->Bind(1);

	SecondSource->GetColorAttachment()->Bind(2);
	SecondSource->GetDepthAttachment()->Bind(3);

	FEShader* CurrentShader = RESOURCE_MANAGER.GetShader("5C267A01466A545E7D1A2E66"/*FECombineFrameBuffers*/);
	CurrentShader->Start();
	CurrentShader->LoadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	CurrentShader->Stop();

	// Return to default value.
	glDepthFunc(GL_LESS);

	FirstSource->GetColorAttachment()->UnBind();
	FirstSource->GetDepthAttachment()->UnBind();

	SecondSource->GetColorAttachment()->UnBind();
	SecondSource->GetDepthAttachment()->UnBind();

	Target->UnBind();

	return true;
}

void FERenderer::AddAfterRenderCallback(std::function<void()> Callback)
{
	if (Callback == nullptr)
	{
		LOG.Add("Attempted to call FERenderer::AddAfterRenderCallback with Callback set to nullptr.", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	AfterRenderCallbacks.push_back(Callback);
}

bool FERenderer::IsClearActiveInSimplifiedRendering()
{
	return bClearActiveInSimplifiedRendering;
}

void FERenderer::SetClearActiveInSimplifiedRendering(bool NewValue)
{
	bClearActiveInSimplifiedRendering = NewValue;
}

FEEntity* FERenderer::TryToGetLastUsedCameraEntity()
{
	if (LastRenderedCameraID.empty())
		return nullptr;
	
	FEObject* LastRenderedCamera = OBJECT_MANAGER.GetFEObject(LastRenderedCameraID);
	if (LastRenderedCamera == nullptr)
	{
		LastRenderedCameraID = "";
		return nullptr;
	}

	return reinterpret_cast<FEEntity*>(LastRenderedCamera);
}

FEFramebuffer* FERenderer::GetLastRenderedResult()
{
	return LastRenderedResult;
}

void FEGBuffer::InitializeResources(FEFramebuffer* MainFrameBuffer)
{
	GFrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());

	Positions = RESOURCE_MANAGER.CreateTexture(GL_RGB32F, GL_RGB, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(Positions, 1);

	Normals = RESOURCE_MANAGER.CreateTexture(GL_RGB16F, GL_RGB, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(Normals, 2);

	Albedo = RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(Albedo, 3);

	MaterialProperties = RESOURCE_MANAGER.CreateTexture(GL_RGBA16F, GL_RGBA, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(MaterialProperties, 4);

	ShaderProperties = RESOURCE_MANAGER.CreateTexture(GL_RGBA, GL_RGBA, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(ShaderProperties, 5);
}

FEGBuffer::FEGBuffer(FEFramebuffer* MainFrameBuffer)
{
	InitializeResources(MainFrameBuffer);
}

void FEGBuffer::RenderTargetResize(FEFramebuffer* MainFrameBuffer)
{
	delete GFrameBuffer;
	InitializeResources(MainFrameBuffer);
}

FESSAO::FESSAO(FEFramebuffer* MainFrameBuffer)
{
	InitializeResources(MainFrameBuffer);
}

void FESSAO::InitializeResources(FEFramebuffer* MainFrameBuffer)
{
	FB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight(), false);
	Shader = RESOURCE_MANAGER.GetShader("1037115B676E383E36345079"/*"FESSAOShader"*/);
}

void FESSAO::RenderTargetResize(FEFramebuffer* MainFrameBuffer)
{
	delete FB;
	InitializeResources(MainFrameBuffer);
}