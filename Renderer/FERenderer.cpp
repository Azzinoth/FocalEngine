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

		SkyDome = RESOURCE_MANAGER.CreateEntity(RESOURCE_MANAGER.GetGameModel("17271E603508013IO77931TY"/*"skyDomeGameModel"*/), "skyDomeEntity");
		RESOURCE_MANAGER.MakePrefabStandard(SkyDome->Prefab);
		SkyDome->bVisible = false;
		SkyDome->Transform.SetScale(glm::vec3(50.0f));

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

void FERenderer::LoadStandardParams(FEShader* Shader, const FEBasicCamera* CurrentCamera, FEMaterial* Material, const FETransformComponent* Transform, const bool IsReceivingShadows, const bool IsUniformLighting)
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

	if (Shader->GetParameter("FEWorldMatrix") != nullptr)
		Shader->UpdateParameterData("FEWorldMatrix", Transform->GetTransformMatrix());

	if (Shader->GetParameter("FEViewMatrix") != nullptr)
		Shader->UpdateParameterData("FEViewMatrix", CurrentCamera->GetViewMatrix());

	if (Shader->GetParameter("FEProjectionMatrix") != nullptr)
		Shader->UpdateParameterData("FEProjectionMatrix", CurrentCamera->GetProjectionMatrix());

	if (Shader->GetParameter("FEPVMMatrix") != nullptr)
		Shader->UpdateParameterData("FEPVMMatrix", CurrentCamera->GetProjectionMatrix() * CurrentCamera->GetViewMatrix() * Transform->GetTransformMatrix());

	if (Shader->GetParameter("FECameraPosition") != nullptr)
		Shader->UpdateParameterData("FECameraPosition", CurrentCamera->GetPosition());

	if (Shader->GetParameter("FEGamma") != nullptr)
		Shader->UpdateParameterData("FEGamma", CurrentCamera->GetGamma());

	if (Shader->GetParameter("FEExposure") != nullptr)
		Shader->UpdateParameterData("FEExposure", CurrentCamera->GetExposure());

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

void FERenderer::LoadStandardParams(FEShader* Shader, const FEBasicCamera* CurrentCamera, const bool IsReceivingShadows, const bool IsUniformLighting)
{
	if (Shader->GetParameter("FEViewMatrix") != nullptr)
		Shader->UpdateParameterData("FEViewMatrix", CurrentCamera->GetViewMatrix());

	if (Shader->GetParameter("FEProjectionMatrix") != nullptr)
		Shader->UpdateParameterData("FEProjectionMatrix", CurrentCamera->GetProjectionMatrix());

	if (Shader->GetParameter("FECameraPosition") != nullptr)
		Shader->UpdateParameterData("FECameraPosition", CurrentCamera->GetPosition());

	if (Shader->GetParameter("FEGamma") != nullptr)
		Shader->UpdateParameterData("FEGamma", CurrentCamera->GetGamma());

	if (Shader->GetParameter("FEExposure") != nullptr)
		Shader->UpdateParameterData("FEExposure", CurrentCamera->GetExposure());

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

void FERenderer::LoadUniformBlocks()
{
	std::vector<FELightShaderInfo> Info;
	Info.resize(FE_MAX_LIGHTS);

	// direction light information for shaders
	FEDirectionalLightShaderInfo DirectionalLightInfo;

	int Index = 0;
	auto LightIterator = OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (LightIterator != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		FEDirectionalLight* Light = reinterpret_cast<FEDirectionalLight*>(LightIterator->second);

		DirectionalLightInfo.Position = glm::vec4(Light->Transform.GetPosition(), 0.0f);
		DirectionalLightInfo.Color = glm::vec4(Light->GetColor() * Light->GetIntensity(), 0.0f);
		DirectionalLightInfo.Direction = glm::vec4(Light->GetDirection(), 0.0f);
		DirectionalLightInfo.CSM0 = Light->CascadeData[0].ProjectionMat * Light->CascadeData[0].ViewMat;
		DirectionalLightInfo.CSM1 = Light->CascadeData[1].ProjectionMat * Light->CascadeData[1].ViewMat;
		DirectionalLightInfo.CSM2 = Light->CascadeData[2].ProjectionMat * Light->CascadeData[2].ViewMat;
		DirectionalLightInfo.CSM3 = Light->CascadeData[3].ProjectionMat * Light->CascadeData[3].ViewMat;
		DirectionalLightInfo.CSMSizes = glm::vec4(Light->CascadeData[0].Size, Light->CascadeData[1].Size, Light->CascadeData[2].Size, Light->CascadeData[3].Size);
		DirectionalLightInfo.ActiveCascades = Light->ActiveCascades;
		DirectionalLightInfo.BiasFixed = Light->ShadowBias;
		if (!Light->bStaticShadowBias)
			DirectionalLightInfo.BiasFixed = -1.0f;
		DirectionalLightInfo.BiasVariableIntensity = Light->ShadowBiasVariableIntensity;
		DirectionalLightInfo.Intensity = Light->GetIntensity();

		Info[Index].Position = glm::vec4(Light->Transform.GetPosition(), 0.0f);
		Info[Index].Color = glm::vec4(Light->GetColor() * Light->GetIntensity(), 0.0f);

		Index++;
		LightIterator++;
	}

	LightIterator = OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].begin();
	while (LightIterator != OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].end())
	{
		FESpotLight* Light = reinterpret_cast<FESpotLight*>(LightIterator->second);

		Info[Index].TypeAndAngles = glm::vec3(Light->GetType(),
											  glm::cos(glm::radians(Light->GetSpotAngle())),
											  glm::cos(glm::radians(Light->GetSpotAngleOuter())));

		Info[Index].Direction = glm::vec4(Light->GetDirection(), 0.0f);

		Info[Index].Position = glm::vec4(Light->Transform.GetPosition(), 0.0f);
		Info[Index].Color = glm::vec4(Light->GetColor() * Light->GetIntensity(), 0.0f);

		Index++;
		LightIterator++;
	}

	LightIterator = OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].begin();
	while (LightIterator != OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].end())
	{
		FEPointLight* Light = reinterpret_cast<FEPointLight*>(LightIterator->second);
		Info[Index].TypeAndAngles = glm::vec3(Light->GetType(), 0.0f, 0.0f);

		Info[Index].Position = glm::vec4(Light->Transform.GetPosition(), 0.0f);
		Info[Index].Color = glm::vec4(Light->GetColor() * Light->GetIntensity(), 0.0f);

		Index++;
		LightIterator++;
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
				const size_t SizeOfFELightShaderInfo = sizeof(FELightShaderInfo) + 4;
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, IteratorBlock->second));

				//FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeOfFELightShaderInfo * info.size(), &info));
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

void FERenderer::RenderEntityInstanced(FEEntityInstanced* EntityInstanced, FEBasicCamera* CurrentCamera, float** Frustum, const bool bShadowMap, const bool bReloadUniformBlocks, const int ComponentIndex)
{
	if (bReloadUniformBlocks)
		LoadUniformBlocks();

	if (ComponentIndex != -1)
	{
		GPUCulling(EntityInstanced, static_cast<int>(ComponentIndex), CurrentCamera);

		FEGameModel* CurrentGameModel = EntityInstanced->Prefab->Components[ComponentIndex]->GameModel;

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
		const FETransformComponent TempTransform = EntityInstanced->Transform.Combine(EntityInstanced->Prefab->Components[ComponentIndex]->Transform);
		LoadStandardParams(CurrentGameModel->GetMaterial()->Shader, CurrentCamera, CurrentGameModel->Material, &TempTransform, EntityInstanced->IsReceivingShadows(), EntityInstanced->IsUniformLighting());
		CurrentGameModel->GetMaterial()->Shader->LoadDataToGPU();

		EntityInstanced->Render(static_cast<int>(ComponentIndex));

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
			LoadStandardParams(CurrentGameModel->GetBillboardMaterial()->Shader, CurrentCamera, CurrentGameModel->GetBillboardMaterial(), &EntityInstanced->Transform, EntityInstanced->IsReceivingShadows(), EntityInstanced->IsUniformLighting());
			CurrentGameModel->GetBillboardMaterial()->Shader->LoadDataToGPU();

			EntityInstanced->RenderOnlyBillbords(CurrentCamera->GetPosition());

			CurrentGameModel->GetBillboardMaterial()->UnBind();
			if (OriginalShader->GetName() == "FEPBRShader")
				CurrentGameModel->GetBillboardMaterial()->Shader = OriginalShader;

			if (bShadowMap)
			{
				CurrentGameModel->SetBillboardMaterial(RegularBillboardMaterial);
			}
		}

		return;
	}

	for (size_t i = 0; i < EntityInstanced->Prefab->Components.size(); i++)
	{
		GPUCulling(EntityInstanced, static_cast<int>(i), CurrentCamera);

		FEGameModel* CurrentGameModel = EntityInstanced->Prefab->Components[i]->GameModel;

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
		FETransformComponent TempTransform = EntityInstanced->Transform.Combine(EntityInstanced->Prefab->Components[i]->Transform);
		LoadStandardParams(CurrentGameModel->GetMaterial()->Shader, CurrentCamera, CurrentGameModel->Material, &TempTransform, EntityInstanced->IsReceivingShadows(), EntityInstanced->IsUniformLighting());
		CurrentGameModel->GetMaterial()->Shader->LoadDataToGPU();

		EntityInstanced->Render(static_cast<int>(i));

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
			LoadStandardParams(CurrentGameModel->GetBillboardMaterial()->Shader, CurrentCamera, CurrentGameModel->GetBillboardMaterial(), &EntityInstanced->Transform, EntityInstanced->IsReceivingShadows(), EntityInstanced->IsUniformLighting());
			CurrentGameModel->GetBillboardMaterial()->Shader->LoadDataToGPU();

			EntityInstanced->RenderOnlyBillbords(CurrentCamera->GetPosition());

			CurrentGameModel->GetBillboardMaterial()->UnBind();
			if (OriginalShader->GetName() == "FEPBRShader")
				CurrentGameModel->GetBillboardMaterial()->Shader = OriginalShader;

			if (bShadowMap)
			{
				CurrentGameModel->SetBillboardMaterial(RegularBillboardMaterial);
			}
		}
	}
}

void FERenderer::SimplifiedRender(FEBasicCamera* CurrentCamera)
{
	CurrentCamera->UpdateFrustumPlanes();

	SceneToTextureFB->Bind();
	//glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	
	if (bClearActiveInSimplifiedRendering)
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	auto EntityIterator = SCENE.EntityMap.begin();
	while (EntityIterator != SCENE.EntityMap.end())
	{
		auto entity = EntityIterator->second;

		if (entity->IsVisible() && entity->IsPostprocessApplied())
		{
			if (entity->GetType() == FE_ENTITY)
			{
				//ForceShader(RESOURCE_MANAGER.GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
				RenderEntityForward(entity, CurrentCamera);
			}
			else if (entity->GetType() == FE_ENTITY_INSTANCED)
			{
				//ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
				//RenderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(entity), CurrentCamera, CurrentCamera->GetFrustumPlanes(), false);
			}
		}

		EntityIterator++;
	}

	// ********* RENDER INSTANCED LINE *********
	/*FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedLineBuffer));
	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, FE_MAX_LINES * sizeof(FELine), this->LinesBuffer.data()));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	InstancedLineShader->Start();
	InstancedLineShader->UpdateParameterData("FEProjectionMatrix", CurrentCamera->GetProjectionMatrix());
	InstancedLineShader->UpdateParameterData("FEViewMatrix", CurrentCamera->GetViewMatrix());
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
	InstancedLineShader->Stop();*/
	// ********* RENDER INSTANCED LINE END *********

	SceneToTextureFB->UnBind();
	FinalScene = SceneToTextureFB->GetColorAttachment();
	FinalScene->Bind();

	// ********* RENDER FRAME BUFFER TO SCREEN *********
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	ScreenQuadShader->Start();
	LoadStandardParams(ScreenQuadShader, CurrentCamera, true);
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

void FERenderer::Render(FEBasicCamera* CurrentCamera)
{
	LastRenderedResult = nullptr;

	if (bVRActive)
		return;

	if (bSimplifiedRendering)
	{
		SimplifiedRender(CurrentCamera);
		return;
	}

	CurrentCamera->UpdateFrustumPlanes();

	LastTestTime = TestTime;
	TestTime = 0.0f;

	// there is only 1 directional light, sun.
	// and we need to set correct light position
	//#fix it should update view matrices for each cascade!

	auto LightIterator = OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (LightIterator != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		FEDirectionalLight* Light = reinterpret_cast<FEDirectionalLight*>(LightIterator->second);
		if (Light->IsCastShadows())
		{
			Light->UpdateCascades(CurrentCamera->Fov, CurrentCamera->AspectRatio,
								  CurrentCamera->NearPlane, CurrentCamera->FarPlane, 
								  CurrentCamera->ViewMatrix, CurrentCamera->GetForward(),
								  CurrentCamera->GetRight(), CurrentCamera->GetUp());
		}
		LightIterator++;
	}

	LoadUniformBlocks();

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

	auto ItLight = OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (ItLight != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		FEDirectionalLight* Light = reinterpret_cast<FEDirectionalLight*>(ItLight->second);
		if (Light->IsCastShadows())
		{
			const float ShadowsBlurFactor = Light->GetShadowBlurFactor();
			ShaderPBR->UpdateParameterData("shadowBlurFactor", ShadowsBlurFactor);
			ShaderInstancedPBR->UpdateParameterData("shadowBlurFactor", ShadowsBlurFactor);

			const glm::vec3 OldCameraPosition = CurrentCamera->GetPosition();
			const glm::mat4 OldViewMatrix = CurrentCamera->GetViewMatrix();
			const glm::mat4 OldProjectionMatrix = CurrentCamera->GetProjectionMatrix();

			for (size_t i = 0; i < static_cast<size_t>(Light->ActiveCascades); i++)
			{
				// put camera to the position of light
				CurrentCamera->ProjectionMatrix = Light->CascadeData[i].ProjectionMat;
				CurrentCamera->ViewMatrix = Light->CascadeData[i].ViewMat;

				FE_GL_ERROR(glViewport(0, 0, Light->CascadeData[i].FrameBuffer->GetWidth(), Light->CascadeData[i].FrameBuffer->GetHeight()));

				UpdateGPUCullingFrustum(Light->CascadeData[i].Frustum, CurrentCamera->GetPosition());

				Light->CascadeData[i].FrameBuffer->Bind();
				FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));

				auto ItTerrain = SCENE.TerrainMap.begin();
				while (ItTerrain != SCENE.TerrainMap.end())
				{
					auto terrain = ItTerrain->second;
					if (!terrain->IsCastingShadows() || !terrain->IsVisible())
					{
						ItTerrain++;
						continue;
					}
						
					terrain->Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
					RenderTerrain(terrain, CurrentCamera);
					terrain->Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
					ItTerrain++;
				}

				auto it = SCENE.EntityMap.begin();
				while (it != SCENE.EntityMap.end())
				{
					const auto Entity = it->second;
					if (!Entity->IsCastShadows() || !Entity->IsVisible() || Entity->Prefab == nullptr)
					{
						it++;
						continue;
					}

					if (Entity->GetType() == FE_ENTITY)
					{
						for (size_t j = 0; j < Entity->Prefab->Components.size(); j++)
						{
							FEMaterial* OriginalMaterial = Entity->Prefab->Components[j]->GameModel->Material;
							Entity->Prefab->Components[j]->GameModel->Material = ShadowMapMaterial;
							ShadowMapMaterial->SetAlbedoMap(OriginalMaterial->GetAlbedoMap());
							// if material have submaterial
							if (OriginalMaterial->GetAlbedoMap(1) != nullptr)
							{
								ShadowMapMaterial->SetAlbedoMap(OriginalMaterial->GetAlbedoMap(1), 1);
								ShadowMapMaterial->GetAlbedoMap(1)->Bind(1);
							}

							RenderEntity(Entity, CurrentCamera, false, static_cast<int>(j));

							Entity->Prefab->Components[j]->GameModel->Material = OriginalMaterial;
							for (size_t k = 0; k < ShadowMapMaterial->Textures.size(); k++)
							{
								ShadowMapMaterial->Textures[k] = nullptr;
								ShadowMapMaterial->TextureBindings[k] = -1;

								ShadowMapMaterialInstanced->Textures[k] = nullptr;
								ShadowMapMaterialInstanced->TextureBindings[k] = -1;
							}
						}
					}
					else if (Entity->GetType() == FE_ENTITY_INSTANCED)
					{
						std::vector<FEMaterial*> OriginalMaterials;
						FEEntityInstanced* CurrentEntity = reinterpret_cast<FEEntityInstanced*>(Entity);
						for (size_t j = 0; j < CurrentEntity->Prefab->Components.size(); j++)
						{
							OriginalMaterials.push_back(CurrentEntity->Prefab->Components[j]->GameModel->Material);

							CurrentEntity->Prefab->Components[j]->GameModel->Material = ShadowMapMaterialInstanced;
							ShadowMapMaterialInstanced->SetAlbedoMap(OriginalMaterials.back()->GetAlbedoMap());
							// if material have submaterial
							if (OriginalMaterials.back()->GetAlbedoMap(1) != nullptr)
							{
								ShadowMapMaterialInstanced->SetAlbedoMap(OriginalMaterials.back()->GetAlbedoMap(1), 1);
								ShadowMapMaterialInstanced->GetAlbedoMap(1)->Bind(1);
							}
							
							RenderEntityInstanced(CurrentEntity, CurrentCamera, Light->CascadeData[i].Frustum, true, false, static_cast<int>(j));
							
							Entity->Prefab->Components[j]->GameModel->Material = OriginalMaterials[j];
							for (size_t k = 0; k < ShadowMapMaterial->Textures.size(); k++)
							{
								ShadowMapMaterial->Textures[k] = nullptr;
								ShadowMapMaterial->TextureBindings[k] = -1;

								ShadowMapMaterialInstanced->Textures[k] = nullptr;
								ShadowMapMaterialInstanced->TextureBindings[k] = -1;
							}
						}
					}
						
					it++;
				}

				Light->CascadeData[i].FrameBuffer->UnBind();
				switch (i)
				{
					case 0: CSM0 = Light->CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					case 1: CSM1 = Light->CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					case 2: CSM2 = Light->CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					case 3: CSM3 = Light->CascadeData[i].FrameBuffer->GetDepthAttachment();
						break;
					default:
						break;
				}
			}

			CurrentCamera->SetPosition(OldCameraPosition);
			CurrentCamera->ViewMatrix = OldViewMatrix;
			CurrentCamera->ProjectionMatrix = OldProjectionMatrix;

			FE_GL_ERROR(glViewport(0, 0, SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight()));
			break;
		}

		ItLight++;
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

	UpdateGPUCullingFrustum(CurrentCamera->Frustum, CurrentCamera->GetPosition());

	auto EntityIterator = SCENE.EntityMap.begin();
	while (EntityIterator != SCENE.EntityMap.end())
	{
		auto entity = EntityIterator->second;

		if (entity->IsVisible() && entity->IsPostprocessApplied())
		{
			if (entity->GetType() == FE_ENTITY)
			{
				ForceShader(RESOURCE_MANAGER.GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
				RenderEntity(entity, CurrentCamera);
			}
			else if (entity->GetType() == FE_ENTITY_INSTANCED)
			{
				ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
				RenderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(entity), CurrentCamera, CurrentCamera->GetFrustumPlanes(), false);
			}
		}

		EntityIterator++;
	}

	// It is not renderer work to update interaction ray.
	// It should be done in the input update.
	auto VirtualUIIterator = SCENE.VirtualUIContextMap.begin();
	while (VirtualUIIterator != SCENE.VirtualUIContextMap.end())
	{
		auto VirtualUIContext = VirtualUIIterator->second;
		if (VirtualUIContext->bMouseMovePassThrough)
			VirtualUIContext->UpdateInteractionRay(CurrentCamera->GetPosition(), RENDERER.MouseRay);

		VirtualUIIterator++;
	}

	auto ItTerrain = SCENE.TerrainMap.begin();
	while (ItTerrain != SCENE.TerrainMap.end())
	{
		auto terrain = ItTerrain->second;
		if (terrain->IsVisible())
			RenderTerrain(terrain, CurrentCamera);

		ItTerrain++;
	}

	GBuffer->GFrameBuffer->UnBind();
	ForceShader(nullptr);

	GBuffer->Albedo->Bind(0);
	GBuffer->Normals->Bind(1);
	GBuffer->MaterialProperties->Bind(2);
	GBuffer->Positions->Bind(3);
	GBuffer->ShaderProperties->Bind(4);
	
	// ************************************ SSAO ************************************
	UpdateSSAO(CurrentCamera);
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
	LoadStandardParams(FinalSceneShader, CurrentCamera, true);
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
	InstancedLineShader->UpdateParameterData("FEProjectionMatrix", CurrentCamera->GetProjectionMatrix());
	InstancedLineShader->UpdateParameterData("FEViewMatrix", CurrentCamera->GetViewMatrix());
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
	if (IsSkyEnabled())
		RenderEntity(SkyDome, CurrentCamera);

	SceneToTextureFB->UnBind();
	// ********* RENDER SCENE END *********

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
			LoadStandardParams(Effect.Stages[j]->Shader, CurrentCamera, nullptr, nullptr);
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

	EntityIterator = SCENE.EntityMap.begin();
	while (EntityIterator != SCENE.EntityMap.end())
	{
		auto Entity = EntityIterator->second;

		if (Entity->IsVisible() && !Entity->IsPostprocessApplied())
		{
			if (Entity->GetType() == FE_ENTITY)
			{
				RenderEntity(Entity, CurrentCamera);
			}
			else if (Entity->GetType() == FE_ENTITY_INSTANCED)
			{
			}
		}

		EntityIterator++;
	}
	
	SceneToTextureFB->UnBind();
	SceneToTextureFB->SetColorAttachment(OriginalColorAttachment);
	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI END *********

	// **************************** TERRAIN EDITOR TOOLS ****************************
	ItTerrain = SCENE.TerrainMap.begin();
	while (ItTerrain != SCENE.TerrainMap.end())
	{
		auto terrain = ItTerrain->second;
		if (terrain->IsVisible())
			UpdateTerrainBrush(terrain);

		ItTerrain++;
	}
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
	std::vector<std::string> result;
	for (size_t i = 0; i < PostProcessEffects.size(); i++)
		result.push_back(PostProcessEffects[i]->GetObjectID());
		
	return result;
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

	//RESOURCE_MANAGER.saveFETexture(fileName, pixels, width, height);
	delete[] pixels;
}

void FERenderer::RenderEntity(const FEEntity* Entity, const FEBasicCamera* CurrentCamera, const bool bReloadUniformBlocks, const int ComponentIndex)
{
	if (Entity->IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (bReloadUniformBlocks)
		LoadUniformBlocks();

	if (ComponentIndex == -1)
	{
		for (size_t i = 0; i < Entity->Prefab->Components.size(); i++)
		{
			if (Entity->Prefab->Components[i]->GameModel == nullptr)
			{
				LOG.Add("Trying to draw Entity with GameModel that is nullptr in FERenderer::RenderEntity", "FE_LOG_RENDERING", FE_LOG_ERROR);
				continue;
			}

			if (Entity->Prefab->Components[i]->GameModel->Material == nullptr)
			{
				LOG.Add("Trying to draw Entity with Material that is nullptr in FERenderer::RenderEntity", "FE_LOG_RENDERING", FE_LOG_ERROR);
				continue;
			}

			if (Entity->Prefab->Components[i]->GameModel->Material->Shader == nullptr)
			{
				LOG.Add("Trying to draw Entity with Shader that is nullptr in FERenderer::RenderEntity", "FE_LOG_RENDERING", FE_LOG_ERROR);
				continue;
			}
				
			FEShader* OriginalShader = Entity->Prefab->Components[i]->GameModel->Material->Shader;
			if (OriginalShader == nullptr)
				continue;

			if (ShaderToForce)
			{
				if (OriginalShader->GetName() == "FEPBRShader")
					Entity->Prefab->Components[i]->GameModel->Material->Shader = ShaderToForce;
			}

			Entity->Prefab->Components[i]->GameModel->Material->Bind();
			FETransformComponent TempTransform;
			if (Entity->Prefab->Components.size() == 1)
			{
				TempTransform = Entity->Transform;
			}
			else
			{
				TempTransform = Entity->Transform.Combine(Entity->Prefab->Components[i]->Transform);
			}
			LoadStandardParams(Entity->Prefab->Components[i]->GameModel->Material->Shader, CurrentCamera, Entity->Prefab->Components[i]->GameModel->Material, &TempTransform, Entity->IsReceivingShadows(), Entity->IsUniformLighting());
			Entity->Prefab->Components[i]->GameModel->Material->Shader->LoadDataToGPU();

			FE_GL_ERROR(glBindVertexArray(Entity->Prefab->Components[i]->GameModel->Mesh->GetVaoID()));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_INDEX) == FE_INDEX)
				FE_GL_ERROR(glDrawElements(GL_TRIANGLES, Entity->Prefab->Components[i]->GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
			if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_INDEX) != FE_INDEX)
				FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, Entity->Prefab->Components[i]->GameModel->Mesh->GetVertexCount()));

			FE_GL_ERROR(glBindVertexArray(0));

			Entity->Prefab->Components[i]->GameModel->Material->UnBind();

			if (ShaderToForce)
			{
				if (OriginalShader->GetName() == "FEPBRShader")
					Entity->Prefab->Components[i]->GameModel->Material->Shader = OriginalShader;
			}
		}
	}
	else
	{
		FEShader* OriginalShader = Entity->Prefab->Components[ComponentIndex]->GameModel->Material->Shader;
		if (ShaderToForce)
		{
			if (OriginalShader->GetName() == "FEPBRShader")
				Entity->Prefab->Components[ComponentIndex]->GameModel->Material->Shader = ShaderToForce;
		}

		Entity->Prefab->Components[ComponentIndex]->GameModel->Material->Bind();
		const FETransformComponent TempTransform = Entity->Transform.Combine(Entity->Prefab->Components[ComponentIndex]->Transform);
		LoadStandardParams(Entity->Prefab->Components[ComponentIndex]->GameModel->Material->Shader, CurrentCamera, Entity->Prefab->Components[ComponentIndex]->GameModel->Material, &TempTransform, Entity->IsReceivingShadows(), Entity->IsUniformLighting());
		Entity->Prefab->Components[ComponentIndex]->GameModel->Material->Shader->LoadDataToGPU();

		FE_GL_ERROR(glBindVertexArray(Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->GetVaoID()));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_INDEX) == FE_INDEX)
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
		if ((Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->VertexAttributes & FE_INDEX) != FE_INDEX)
			FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, Entity->Prefab->Components[ComponentIndex]->GameModel->Mesh->GetVertexCount()));

		FE_GL_ERROR(glBindVertexArray(0));

		Entity->Prefab->Components[ComponentIndex]->GameModel->Material->UnBind();

		if (ShaderToForce)
		{
			if (OriginalShader->GetName() == "FEPBRShader")
				Entity->Prefab->Components[ComponentIndex]->GameModel->Material->Shader = OriginalShader;
		}
	}

	if (Entity->IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FERenderer::RenderEntityForward(const FEEntity* Entity, const FEBasicCamera* CurrentCamera, const bool bReloadUniformBlocks)
{
	if (bReloadUniformBlocks)
		LoadUniformBlocks();

	for (size_t i = 0; i < Entity->Prefab->Components.size(); i++)
	{
		FEShader* OriginalShader = nullptr;
		if (!bSimplifiedRendering || RENDERER.bVRActive)
		{
			OriginalShader = Entity->Prefab->Components[i]->GameModel->Material->Shader;
			if (RENDERER.bVRActive)
			{
				if (OriginalShader->GetObjectID() != "6917497A5E0C05454876186F"/*"SolidColorMaterial"*/)
				Entity->Prefab->Components[i]->GameModel->Material->Shader = RESOURCE_MANAGER.GetShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
			}
			else
			{
				Entity->Prefab->Components[i]->GameModel->Material->Shader = RESOURCE_MANAGER.GetShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
			}
		}

		Entity->Prefab->Components[i]->GameModel->Material->Bind();
		LoadStandardParams(Entity->Prefab->Components[i]->GameModel->Material->Shader, CurrentCamera, Entity->Prefab->Components[i]->GameModel->Material, &Entity->Transform, Entity->IsReceivingShadows(), Entity->IsUniformLighting());
		Entity->Prefab->Components[i]->GameModel->Material->Shader->LoadDataToGPU();

		FE_GL_ERROR(glBindVertexArray(Entity->Prefab->Components[i]->GameModel->Mesh->GetVaoID()));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_INDEX) == FE_INDEX)
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, Entity->Prefab->Components[i]->GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
		if ((Entity->Prefab->Components[i]->GameModel->Mesh->VertexAttributes & FE_INDEX) != FE_INDEX)
			FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, Entity->Prefab->Components[i]->GameModel->Mesh->GetVertexCount()));

		FE_GL_ERROR(glBindVertexArray(0));

		Entity->Prefab->Components[i]->GameModel->Material->UnBind();

		if (!bSimplifiedRendering || RENDERER.bVRActive)
			Entity->Prefab->Components[i]->GameModel->Material->Shader = OriginalShader;
	}
}

void FERenderer::RenderTerrain(FETerrain* Terrain, const FEBasicCamera* CurrentCamera)
{
	if (Terrain->IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (Terrain->Shader->GetName() == "FESMTerrainShader")
	{
		Terrain->HeightMap->Bind(0);
	}
	else
	{
		for (size_t i = 0; i < Terrain->Layers.size(); i++)
		{
			if (Terrain->Layers[i] != nullptr && Terrain->Layers[i]->GetMaterial()->IsCompackPacking())
			{
				if (Terrain->Layers[i]->GetMaterial()->GetAlbedoMap() != nullptr)
					Terrain->Layers[i]->GetMaterial()->GetAlbedoMap()->Bind(static_cast<int>(i * 3));

				if (Terrain->Layers[i]->GetMaterial()->GetNormalMap() != nullptr)
					Terrain->Layers[i]->GetMaterial()->GetNormalMap()->Bind(static_cast<int>(i * 3 + 1));

				if (Terrain->Layers[i]->GetMaterial()->GetAOMap() != nullptr)
					Terrain->Layers[i]->GetMaterial()->GetAOMap()->Bind(static_cast<int>(i * 3 + 2));
			}
		}

		Terrain->HeightMap->Bind(24);
		if (Terrain->ProjectedMap != nullptr)
			Terrain->ProjectedMap->Bind(25);

		for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE; i++)
		{
			if (Terrain->LayerMaps[i] != nullptr)
				Terrain->LayerMaps[i]->Bind(static_cast<int>(26 + i));
		}
	}

	Terrain->Shader->Start();
	LoadStandardParams(Terrain->Shader, CurrentCamera, nullptr, &Terrain->Transform, Terrain->IsReceivingShadows());
	// ************ Load materials data for all terrain layers ************

	const int LayersUsed = Terrain->LayersUsed();
	if (LayersUsed == 0)
	{
		// 0 index is for hightMap.
		RESOURCE_MANAGER.NoTexture->Bind(1);
	}

	Terrain->LoadLayersDataToGPU();
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Terrain->GPULayersDataBuffer));

	// Shadow map shader does not have this parameter.
	if (Terrain->Shader->GetParameter("usedLayersCount") != nullptr)
		Terrain->Shader->UpdateParameterData("usedLayersCount", static_cast<float>(LayersUsed));
	// ************ Load materials data for all terrain layers END ************

	Terrain->Shader->UpdateParameterData("hightScale", Terrain->HightScale);
	Terrain->Shader->UpdateParameterData("scaleFactor", Terrain->ScaleFactor);
	if (Terrain->Shader->GetName() != "FESMTerrainShader")
		Terrain->Shader->UpdateParameterData("tileMult", Terrain->TileMult);
	Terrain->Shader->UpdateParameterData("LODlevel", Terrain->LODLevel);
	Terrain->Shader->UpdateParameterData("hightMapShift", Terrain->HightMapShift);

	static glm::vec3 PivotPosition = Terrain->Transform.GetPosition();
	PivotPosition = Terrain->Transform.GetPosition();
	Terrain->ScaleFactor = 1.0f * Terrain->ChunkPerSide;

	static int PVMHash = static_cast<int>(std::hash<std::string>{}("FEPVMMatrix"));
	static int WorldMatrixHash = static_cast<int>(std::hash<std::string>{}("FEWorldMatrix"));
	static int HightMapShiftHash = static_cast<int>(std::hash<std::string>{}("hightMapShift"));

	const bool bWasDirty = Terrain->Transform.bDirtyFlag;
	Terrain->Shader->LoadDataToGPU();
	for (size_t i = 0; i < Terrain->ChunkPerSide; i++)
	{
		for (size_t j = 0; j < Terrain->ChunkPerSide; j++)
		{
			Terrain->Transform.SetPosition(glm::vec3(PivotPosition.x + i * 64.0f * Terrain->Transform.Scale[0], PivotPosition.y, PivotPosition.z + j * 64.0f * Terrain->Transform.Scale[2]));

			Terrain->Shader->UpdateParameterData("FEPVMMatrix", CurrentCamera->GetProjectionMatrix() * CurrentCamera->GetViewMatrix() * Terrain->Transform.GetTransformMatrix());
			if (Terrain->Shader->GetParameter("FEWorldMatrix") != nullptr)
				Terrain->Shader->UpdateParameterData("FEWorldMatrix", Terrain->Transform.GetTransformMatrix());
			Terrain->Shader->UpdateParameterData("hightMapShift", glm::vec2(i * -1.0f, j * -1.0f));

			Terrain->Shader->LoadMatrix(PVMHash, *static_cast<glm::mat4*>(Terrain->Shader->GetParameter("FEPVMMatrix")->Data));
			if (Terrain->Shader->GetParameter("FEWorldMatrix") != nullptr)
				Terrain->Shader->LoadMatrix(WorldMatrixHash, *static_cast<glm::mat4*>(Terrain->Shader->GetParameter("FEWorldMatrix")->Data));

			if (Terrain->Shader->GetParameter("hightMapShift") != nullptr)
				Terrain->Shader->LoadVector(HightMapShiftHash, *static_cast<glm::vec2*>(Terrain->Shader->GetParameter("hightMapShift")->Data));

			Terrain->Render();
		}
	}
	Terrain->Shader->Stop();
	Terrain->Transform.SetPosition(PivotPosition);

	if (!bWasDirty)
		Terrain->Transform.bDirtyFlag = false;

	if (Terrain->IsWireframeMode())
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

void FERenderer::UpdateTerrainBrush(FETerrain* Terrain)
{
	Terrain->UpdateBrush(EngineMainCamera->Position, MouseRay);
	FE_GL_ERROR(glViewport(0, 0, SceneToTextureFB->GetWidth(), SceneToTextureFB->GetHeight()));
}

bool FERenderer::IsSkyEnabled()
{
	return SkyDome->IsVisible();
}

void FERenderer::SetSkyEnabled(const bool NewValue)
{
	SkyDome->SetVisibility(NewValue);
}

float FERenderer::GetDistanceToSky()
{
	return SkyDome->Transform.Scale[0];
}

void FERenderer::SetDistanceToSky(const float NewValue)
{
	SkyDome->Transform.SetScale(glm::vec3(NewValue));
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

void FERenderer::UpdateGPUCullingFrustum(float** Frustum, glm::vec3 CameraPosition)
{
	float* FrustumBufferData = static_cast<float*>(glMapNamedBufferRange(FrustumInfoBuffer, 0, sizeof(float) * (32),
	                                                                     GL_MAP_WRITE_BIT |
	                                                                     GL_MAP_INVALIDATE_BUFFER_BIT |
	                                                                     GL_MAP_UNSYNCHRONIZED_BIT));

	for (size_t i = 0; i < 6; i++)
	{
		FrustumBufferData[i * 4] = Frustum[i][0];
		FrustumBufferData[i * 4 + 1] = Frustum[i][1];
		FrustumBufferData[i * 4 + 2] = Frustum[i][2];
		FrustumBufferData[i * 4 + 3] = Frustum[i][3];
	}

	FrustumBufferData[24] = CameraPosition[0];
	FrustumBufferData[25] = CameraPosition[1];
	FrustumBufferData[26] = CameraPosition[2];

	FE_GL_ERROR(glUnmapNamedBuffer(FrustumInfoBuffer));
}

void FERenderer::GPUCulling(FEEntityInstanced* Entity, const int SubGameModel, const FEBasicCamera* CurrentCamera)
{
	if (bFreezeCulling)
		return;

	Entity->CheckDirtyFlag(SubGameModel);

	FrustumCullingShader->Start();

#ifdef USE_OCCLUSION_CULLING
	FrustumCullingShader->UpdateParameterData("FEProjectionMatrix", CurrentCamera->GetProjectionMatrix());
	FrustumCullingShader->UpdateParameterData("FEViewMatrix", CurrentCamera->GetViewMatrix());
	FrustumCullingShader->UpdateParameterData("useOcclusionCulling", bUseOcclusionCulling);
	// It should be last frame size!
	const glm::vec2 RenderTargetSize = glm::vec2(GBuffer->GFrameBuffer->DepthAttachment->GetWidth(), GBuffer->GFrameBuffer->DepthAttachment->GetHeight());
	FrustumCullingShader->UpdateParameterData("renderTargetSize", RenderTargetSize);
	FrustumCullingShader->UpdateParameterData("nearFarPlanes", glm::vec2(CurrentCamera->NearPlane, CurrentCamera->FarPlane));
#endif // USE_OCCLUSION_CULLING

	FrustumCullingShader->LoadDataToGPU();

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Entity->Renderers[SubGameModel]->SourceDataBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Entity->Renderers[SubGameModel]->PositionsBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FrustumInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Entity->Renderers[SubGameModel]->LODBuffers[0]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, Entity->Renderers[SubGameModel]->AABBSizesBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, CullingLODCountersBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, Entity->Renderers[SubGameModel]->LODInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, Entity->Renderers[SubGameModel]->LODBuffers[1]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, Entity->Renderers[SubGameModel]->LODBuffers[2]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, Entity->Renderers[SubGameModel]->LODBuffers[3]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, Entity->Renderers[SubGameModel]->IndirectDrawInfoBuffer));

	DepthPyramid->Bind(0);
	/*FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, depthPyramid));*/

	FrustumCullingShader->Dispatch(static_cast<GLuint>(ceil(Entity->InstanceCount / 64.0f)), 1, 1);
	FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));

	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, 0));
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

void FERenderer::UpdateSSAO(const FEBasicCamera* CurrentCamera)
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
	LoadStandardParams(SSAO->Shader, CurrentCamera, true);
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

void FERenderer::RenderVR(FEBasicCamera* CurrentCamera/*, uint32_t ColorTexture, uint32_t DepthTexture*/)
{
	if (SceneToVRTextureFB == nullptr)
		return;

	// glViewPort and Frame buffer already are set.
	CurrentCamera->UpdateFrustumPlanes();
	FEScene& scene = SCENE;

	SceneToVRTextureFB->Bind();
	glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	LoadUniformBlocks();

	auto EntityIterator = scene.EntityMap.begin();
	while (EntityIterator != scene.EntityMap.end())
	{
		auto entity = EntityIterator->second;

		if (entity->IsVisible() && entity->IsPostprocessApplied())
		{
			if (entity->GetType() == FE_ENTITY)
			{
				//ForceShader(RESOURCE_MANAGER.GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
				RenderEntityForward(entity, CurrentCamera);
			}
			else if (entity->GetType() == FE_ENTITY_INSTANCED)
			{
				//ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
				//RenderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(entity), CurrentCamera, CurrentCamera->GetFrustumPlanes(), false);
			}
		}

		EntityIterator++;
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