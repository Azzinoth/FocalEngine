#include "FERenderer.h"
#include "../FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetRenderer()
{
	return FERenderer::GetInstancePointer();
}
#endif

EntityBasedEngineProvidedData FERenderer::CurrentEntityBasedEngineProvidedData = EntityBasedEngineProvidedData();
CameraBasedEngineProvidedData FERenderer::CurrentCameraBasedEngineProvidedData = CameraBasedEngineProvidedData();

FERenderer::FERenderer()
{
}

void FERenderer::Init()
{
	FEShader* FEScreenQuadShader = RESOURCE_MANAGER.CreateShader("FEScreenQuadShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl")).c_str(),
																					   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_FS.glsl")).c_str(),
																					   nullptr, nullptr, nullptr, nullptr,
																					   "7933272551311F3A1A5B2363");

	RESOURCE_MANAGER.SetTagInternal(FEScreenQuadShader, ENGINE_RESOURCE_TAG);

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
	DebugLines.resize(FE_MAX_DEBUG_LINES);
	DebugLinesDirtyFlags.resize(FE_MAX_DEBUG_LINES);

	const float QuadVertices[] = {
		0.0f,  -0.5f,  0.0f,
		1.0f,  -0.5f,  1.0f,
		1.0f,  0.5f,   1.0f,

		0.0f,  -0.5f,  0.0f,
		1.0f,  0.5f,   1.0f,
		0.0f,  0.5f,   0.0f,
	};
	glGenVertexArrays(1, &DebugLinesVAO);
	glBindVertexArray(DebugLinesVAO);

	unsigned int QuadVBO;
	glGenBuffers(1, &QuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

	glGenBuffers(1, &DebugLinesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, DebugLinesBuffer);
	glBufferData(GL_ARRAY_BUFFER, DebugLines.size() * sizeof(FELine), DebugLines.data(), GL_DYNAMIC_DRAW);

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
														 nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//ComputeShaders//FE_FrustumCulling_CS.glsl")).c_str());

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
														nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//ComputeShaders//FE_ComputeTextureCopy_CS.glsl")).c_str());


	ComputeDepthPyramidDownSample = RESOURCE_MANAGER.CreateShader("FE_ComputeDepthPyramidDownSample",
																	nullptr, nullptr,
																	nullptr, nullptr,
																	nullptr, RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//ComputeShaders//FE_ComputeDepthPyramidDownSample_CS.glsl")).c_str());

	ComputeDepthPyramidDownSample->UpdateUniformData("scaleDownBy", 2);

	FEPostProcess::ScreenQuad = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	FEPostProcess::ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);

	FEShader* BloomThresholdShader = RESOURCE_MANAGER.CreateShader("FEBloomThreshold", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl")).c_str(),
																						RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomThreshold_FS.glsl")).c_str(),
																						nullptr, nullptr, nullptr, nullptr,
																						"0C19574118676C2E5645200E");
	RESOURCE_MANAGER.SetTagInternal(BloomThresholdShader, ENGINE_RESOURCE_TAG);

	FEShader* BloomBlurShader = RESOURCE_MANAGER.CreateShader("FEBloomBlur", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl")).c_str(),
																				RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomBlur_FS.glsl")).c_str(),
																				nullptr, nullptr, nullptr, nullptr,
																				"7F3E4F5C130B537F0846274F");
	RESOURCE_MANAGER.SetTagInternal(BloomBlurShader, ENGINE_RESOURCE_TAG);

	FEShader* BloomCompositionShader = RESOURCE_MANAGER.CreateShader("FEBloomComposition", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl")).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomComposition_FS.glsl")).c_str(),
																							nullptr, nullptr, nullptr, nullptr,
																							"1833272551376C2E5645200E");
	RESOURCE_MANAGER.SetTagInternal(BloomCompositionShader, ENGINE_RESOURCE_TAG);

	FEShader* GammaHDRShader = RESOURCE_MANAGER.CreateShader("FEGammaAndHDRShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_VS.glsl")).c_str(),
																					RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_FS.glsl")).c_str(),
																					nullptr, nullptr, nullptr, nullptr,
																					"3417497A5E0C0C2A07456E44");
	RESOURCE_MANAGER.SetTagInternal(GammaHDRShader, ENGINE_RESOURCE_TAG);

	FEShader* FEFXAAShader = RESOURCE_MANAGER.CreateShader("FEFXAAShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_VS.glsl")).c_str(),
																			RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_FS.glsl")).c_str(),
																			nullptr, nullptr, nullptr, nullptr,
																			"1E69744A10604C2A1221426B");
	RESOURCE_MANAGER.SetTagInternal(FEFXAAShader, ENGINE_RESOURCE_TAG);

	FEShader* DOFShader = RESOURCE_MANAGER.CreateShader("DOF", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_VS.glsl")).c_str(),
																RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_FS.glsl")).c_str(),
																nullptr, nullptr, nullptr, nullptr,
																"7800253C244442155D0F3C7B");
	RESOURCE_MANAGER.SetTagInternal(DOFShader, ENGINE_RESOURCE_TAG);

	FEShader* ChromaticAberrationShader = RESOURCE_MANAGER.CreateShader("chromaticAberrationShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_VS.glsl")).c_str(),
																										RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_FS.glsl")).c_str(),
																										nullptr, nullptr, nullptr, nullptr,
																										"9A41665B5E2B05321A332D09");
	RESOURCE_MANAGER.SetTagInternal(ChromaticAberrationShader, ENGINE_RESOURCE_TAG);

	FEShader* FESSAOShader = RESOURCE_MANAGER.CreateShader("FESSAOShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_VS.glsl")).c_str(),
																			RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_FS.glsl")).c_str(),
																			nullptr, nullptr, nullptr, nullptr,
																			"1037115B676E383E36345079");

	RESOURCE_MANAGER.SetTagInternal(FESSAOShader, ENGINE_RESOURCE_TAG);

	FEShader* FESSAOBlurShader = RESOURCE_MANAGER.CreateShader("FESSAOBlurShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl")).c_str(),
																					RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_Blur_FS.glsl")).c_str(),
																					nullptr, nullptr, nullptr, nullptr,
																					"0B5770660B6970800D776542");
	RESOURCE_MANAGER.SetTagInternal(FESSAOBlurShader, ENGINE_RESOURCE_TAG);

	RENDERER.ShadowMapMaterial = RESOURCE_MANAGER.CreateMaterial("shadowMapMaterial", "7C41565B2E2B05321A182D89" /*"FEShadowMapShader"*/);
	RENDERER.ShadowMapMaterial->Shader = RESOURCE_MANAGER.CreateShader("FEShadowMapShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_VS.glsl")).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl")).c_str());

	RESOURCE_MANAGER.SetTagInternal(RENDERER.ShadowMapMaterial->Shader, ENGINE_RESOURCE_TAG);
	RESOURCE_MANAGER.SetTagInternal(RENDERER.ShadowMapMaterial, ENGINE_RESOURCE_TAG);

	RENDERER.ShadowMapMaterialInstanced = RESOURCE_MANAGER.CreateMaterial("shadowMapMaterialInstanced", "5634765B2E2A05321A182D1A"/*"FEShadowMapShaderInstanced"*/);
	RENDERER.ShadowMapMaterialInstanced->Shader = RESOURCE_MANAGER.CreateShader("FEShadowMapShaderInstanced", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_INSTANCED_VS.glsl")).c_str(),
																											  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl")).c_str());

	RESOURCE_MANAGER.SetTagInternal(RENDERER.ShadowMapMaterialInstanced->Shader, ENGINE_RESOURCE_TAG);
	RESOURCE_MANAGER.SetTagInternal(RENDERER.ShadowMapMaterialInstanced, ENGINE_RESOURCE_TAG);

	FEShader* FESceneFusionShader = RESOURCE_MANAGER.CreateShader("FESceneFusionShader",
																  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl")).c_str(),
																  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_SceneFusion_FS.glsl")).c_str(),
																  nullptr, nullptr,
																  nullptr, nullptr);

	FEShader* FuseFrameBufferDataAndCameraDataDeferred = RESOURCE_MANAGER.CreateShader("FE_FuseFrameBufferDataAndCameraDataDeferred",
																					   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl")).c_str(),
																					   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_FuseFrameBufferDataAndCameraDataDeferred_FS.glsl")).c_str(),
																					   nullptr, nullptr,
																					   nullptr, nullptr);

	FEShader* FESceneFusionShaderDeferred = RESOURCE_MANAGER.CreateShader("FE_SceneFusionDeferred",
																		  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl")).c_str(),
																		  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_SceneFusionDeferred_FS.glsl")).c_str(),
																		  nullptr, nullptr,
																		  nullptr, nullptr);


	FEShader* FEFuseTwoFrameBuffers = RESOURCE_MANAGER.CreateShader("FE_FuseTwoFrameBuffers",
																	RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl")).c_str(),
																	RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.GetEngineFolder() + "CoreExtensions//PostProcessEffects//FE_FuseTwoFrameBuffers_FS.glsl")).c_str(),
																	nullptr, nullptr,
																	nullptr, nullptr); 
}

void FERenderer::LoadStandardUniforms(FEShader* Shader, FEMaterial* Material, FETransformComponent* Transform, FEEntity* Camera, const bool bReceivingShadows, const bool IsUniformLighting)
{
	if (Camera != nullptr)
	{
		FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
		FETransformComponent& CurrentCameraTransformComponent = Camera->GetComponent<FETransformComponent>();

		if (Shader->GetUniform("FEViewMatrix") != nullptr)
			Shader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());

		if (Shader->GetUniform("FEProjectionMatrix") != nullptr)
			Shader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());

		if (Shader->GetUniform("FEPreviousFrameViewMatrix") != nullptr)
			Shader->UpdateUniformData("FEPreviousFrameViewMatrix", CurrentCameraComponent.PreviousFrameViewMatrix);

		if (Shader->GetUniform("ScreenSize") != nullptr)
			Shader->UpdateUniformData("ScreenSize", glm::vec2(CurrentCameraComponent.GetRenderTargetWidth(), CurrentCameraComponent.GetRenderTargetHeight()));

		if (Shader->GetUniform("FEPVMMatrix") != nullptr)
			Shader->UpdateUniformData("FEPVMMatrix", CurrentCameraComponent.GetProjectionMatrix() * CurrentCameraComponent.GetViewMatrix() * Transform->GetWorldMatrix());

		if (Shader->GetUniform("FECameraPosition") != nullptr)
			Shader->UpdateUniformData("FECameraPosition", CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE));

		if (Shader->GetUniform("FECameraDirection") != nullptr)
			Shader->UpdateUniformData("FECameraDirection", CurrentCameraComponent.GetForward());

		if (Shader->GetUniform("FEGamma") != nullptr)
			Shader->UpdateUniformData("FEGamma", CurrentCameraComponent.GetGamma());

		if (Shader->GetUniform("FEExposure") != nullptr)
			Shader->UpdateUniformData("FEExposure", CurrentCameraComponent.GetExposure());
	}
	
	// FE_TO_DO: Maybe it should be removed from here, because material->bind() should handle it.
	if (Shader->GetUniform("textureBindings") != nullptr)
		Shader->GetUniform("textureBindings")->SetValue<std::vector<int>>(Material->TextureBindings);

	// FE_TO_DO: Maybe it should be removed from here, because material->bind() should handle it.
	if (Shader->GetUniform("textureChannels") != nullptr)
		Shader->GetUniform("textureChannels")->SetValue<std::vector<int>>(Material->TextureChannels);

	if (Shader->GetUniform("FEWorldMatrix") != nullptr)
		Shader->UpdateUniformData("FEWorldMatrix", Transform->GetWorldMatrix());

	if (Shader->GetUniform("FEReceiveShadows") != nullptr)
		Shader->UpdateUniformData("FEReceiveShadows", bReceivingShadows);

	if (Shader->GetUniform("FEUniformLighting") != nullptr)
		Shader->UpdateUniformData("FEUniformLighting", IsUniformLighting);

	// FE_TO_DO: Maybe it should be removed from here, because material->bind() should handle it.
	if (Material != nullptr)
	{
		if (Shader->GetUniform("FEAOIntensity") != nullptr)
			Shader->UpdateUniformData("FEAOIntensity", Material->GetAmbientOcclusionIntensity());

		if (Shader->GetUniform("FEAOMapIntensity") != nullptr)
			Shader->UpdateUniformData("FEAOMapIntensity", Material->GetAmbientOcclusionMapIntensity());

		if (Shader->GetUniform("FENormalMapIntensity") != nullptr)
			Shader->UpdateUniformData("FENormalMapIntensity", Material->GetNormalMapIntensity());

		if (Shader->GetUniform("FERoughness") != nullptr)
			Shader->UpdateUniformData("FERoughness", Material->Roughness);

		if (Shader->GetUniform("FERoughnessMapIntensity") != nullptr)
			Shader->UpdateUniformData("FERoughnessMapIntensity", Material->GetRoughnessMapIntensity());

		if (Shader->GetUniform("FEMetalness") != nullptr)
			Shader->UpdateUniformData("FEMetalness", Material->Metalness);

		if (Shader->GetUniform("FEMetalnessMapIntensity") != nullptr)
			Shader->UpdateUniformData("FEMetalnessMapIntensity", Material->GetMetalnessMapIntensity());

		if (Shader->GetUniform("FETiling") != nullptr)
			Shader->UpdateUniformData("FETiling", Material->GetTiling());

		if (Shader->GetUniform("compactMaterialPacking") != nullptr)
			Shader->UpdateUniformData("compactMaterialPacking", Material->IsCompactPacking());
	}
}

void FERenderer::LoadStandardUniforms(FEShader* Shader, const bool bReceivingShadows, FEEntity* Camera, const bool bUniformLighting)
{
	if (Camera != nullptr)
	{
		FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
		FETransformComponent& CurrentCameraTransformComponent = Camera->GetComponent<FETransformComponent>();

		if (Shader->GetUniform("FEViewMatrix") != nullptr)
			Shader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());

		if (Shader->GetUniform("FEProjectionMatrix") != nullptr)
			Shader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());

		if (Shader->GetUniform("FECameraPosition") != nullptr)
			Shader->UpdateUniformData("FECameraPosition", CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE));

		if (Shader->GetUniform("FECameraDirection") != nullptr)
			Shader->UpdateUniformData("FECameraDirection", CurrentCameraComponent.GetForward());

		if (Shader->GetUniform("FEGamma") != nullptr)
			Shader->UpdateUniformData("FEGamma", CurrentCameraComponent.GetGamma());

		if (Shader->GetUniform("FEExposure") != nullptr)
			Shader->UpdateUniformData("FEExposure", CurrentCameraComponent.GetExposure());
	}

	if (Shader->GetUniform("FEReceiveShadows") != nullptr)
		Shader->UpdateUniformData("FEReceiveShadows", bReceivingShadows);

	if (Shader->GetUniform("FEUniformLighting") != nullptr)
		Shader->UpdateUniformData("FEUniformLighting", bUniformLighting);
}

void FERenderer::AddPostProcess(FECameraRenderingData* CameraRenderingData, FEPostProcess* NewPostProcess, const bool bNoProcessing)
{
	if (CameraRenderingData == nullptr)
		return;

	CameraRenderingData->PostProcessEffects.push_back(NewPostProcess);

	if (bNoProcessing)
		return;

	for (size_t i = 0; i < CameraRenderingData->PostProcessEffects.back()->Stages.size(); i++)
	{
		CameraRenderingData->PostProcessEffects.back()->Stages[i]->InTexture.resize(CameraRenderingData->PostProcessEffects.back()->Stages[i]->InTextureSource.size());
		//to-do: change when out texture could be different resolution or/and format.
		//#fix
		if (i == CameraRenderingData->PostProcessEffects.back()->Stages.size() - 1)
		{
			CameraRenderingData->PostProcessEffects.back()->Stages[i]->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(CameraRenderingData->SceneToTextureFB->GetColorAttachment());
		}
		else
		{
			const int FinalW = CameraRenderingData->PostProcessEffects.back()->ScreenWidth;
			const int FinalH = CameraRenderingData->PostProcessEffects.back()->ScreenHeight;
			CameraRenderingData->PostProcessEffects.back()->Stages[i]->OutTexture = RESOURCE_MANAGER.CreateSameFormatTexture(CameraRenderingData->SceneToTextureFB->GetColorAttachment(), FinalW, FinalH);
		}

		CameraRenderingData->PostProcessEffects.back()->TexturesToDelete.push_back(CameraRenderingData->PostProcessEffects.back()->Stages[i]->OutTexture);
	}
}

void FERenderer::LoadUniformBlocks(FEScene* CurrentScene)
{
	std::vector<FELightShaderInfo> Info;
	Info.resize(FE_MAX_LIGHTS);

	// direction light information for shaders
	FEDirectionalLightShaderInfo DirectionalLightInfo;

	int Index = 0;
	std::vector< std::string> LightsIDList = CurrentScene->GetEntityIDListWithComponent<FELightComponent>();
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
	const std::vector<std::string> ShaderList = RESOURCE_MANAGER.GetEnginePrivateShaderIDList();
	for (size_t i = 0; i < ShaderList.size(); i++)
	{
		FEShader* Shader = RESOURCE_MANAGER.GetShader(ShaderList[i]);
		auto IteratorBlock = Shader->BlockUniforms.begin();
		while (IteratorBlock != Shader->BlockUniforms.end())
		{
			if (IteratorBlock->first == LightInfoHash)
			{
				// If shader uniform block was not assigned yet.
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

void FERenderer::RenderGameModelComponentWithInstanced(FEEntity* Entity, FEEntity* Camera, bool bShadowMap, bool bReloadUniformBlocks, size_t PrefabIndex)
{
	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	if (!Entity->IsVisible())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	FEEntity* EntityWithGameModel = INSTANCED_RENDERING_SYSTEM.GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[PrefabIndex]->EntityIDWithGameModelComponent);
	if (EntityWithGameModel == nullptr)
		return;

	FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();

	if (Camera == nullptr)
		return;

	if (bReloadUniformBlocks)
		LoadUniformBlocks(Entity->ParentScene);

	GPUCulling(Entity, GameModelComponent, Camera, PrefabIndex);

	FEGameModel* CurrentGameModel = GameModelComponent.GetGameModel();
	FEShader* OriginalShader = CurrentGameModel->GetMaterial()->Shader;
	if (OriginalShader->GetName() == "FEPBRShader" || OriginalShader->GetName() == "FESolidColorShader")
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
	LoadStandardUniforms(CurrentGameModel->GetMaterial()->Shader, CurrentGameModel->Material, &TransformComponent, Camera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	CurrentGameModel->GetMaterial()->Shader->LoadUniformsDataToGPU();

	INSTANCED_RENDERING_SYSTEM.Render(Entity, GameModelComponent, PrefabIndex);

	CurrentGameModel->GetMaterial()->UnBind();
	if (OriginalShader->GetName() == "FEPBRShader" || OriginalShader->GetName() == "FESolidColorShader")
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
		LoadStandardUniforms(CurrentGameModel->GetBillboardMaterial()->Shader, CurrentGameModel->GetBillboardMaterial(), &TransformComponent, Camera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
		CurrentGameModel->GetBillboardMaterial()->Shader->LoadUniformsDataToGPU();

		INSTANCED_RENDERING_SYSTEM.RenderOnlyBillbords(Entity, GameModelComponent, PrefabIndex);

		CurrentGameModel->GetBillboardMaterial()->UnBind();
		if (OriginalShader->GetName() == "FEPBRShader")
			CurrentGameModel->GetBillboardMaterial()->Shader = OriginalShader;

		if (bShadowMap)
		{
			CurrentGameModel->SetBillboardMaterial(RegularBillboardMaterial);
		}
	}
}

void FERenderer::SimplifiedRender(FEScene* CurrentScene, FEEntity* MainCameraEntity, FECameraRenderingData* CurrentCameraRenderingData)
{
	this->CurrentCameraRenderingData = CurrentCameraRenderingData;
	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();
	CurrentCameraComponent.UpdateFrustum();

	CurrentCameraRenderingData->SceneToTextureFB->Bind();
	SetGLViewport(0, 0, CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight());

	if (CurrentCameraComponent.IsClearColorEnabled())
	{
		glm::vec4 ClearColor = CurrentCameraComponent.GetClearColor();
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	LoadUniformBlocks(CurrentScene);

	entt::basic_group GameModelGroup = CurrentScene->Registry.group<FEGameModelComponent>(entt::get<FETransformComponent>);
	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::GAME_MODEL) /*|| !GameModelComponent.IsPostprocessApplied()*/)
			continue;

		SetEntityForRendering(Entity);
		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			FEMaterial* Material = GameModelComponent.GetGameModel()->GetMaterial();
			FEShader* OriginalShader = Material->Shader;
			if (OriginalShader->GetName() == "FEPBRShader")
				Material->Shader = RESOURCE_MANAGER.GetShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
			
			RenderGameModelComponentForward(Entity, MainCameraEntity);

			if (OriginalShader->GetName() == "FEPBRShader")
				Material->Shader = OriginalShader;
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
			RenderGameModelComponentWithInstanced(Entity, MainCameraEntity);
		}
	}

	entt::basic_view LinesView = CurrentScene->Registry.view<FELineComponent, FETransformComponent>();
	for (auto [EnTTEntity, LineComponent, TransformComponent] : LinesView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		SetEntityForRendering(Entity);
		LINE_SYSTEM.Render(Entity, MainCameraEntity);
	}

	RenderDebugLines(CurrentScene, MainCameraEntity, CurrentCameraRenderingData);

	entt::basic_view VirtualUIView = CurrentScene->Registry.view<FEVirtualUIComponent, FETransformComponent>();
	for (auto [EnTTEntity, VirtualUIComponent, TransformComponent] : VirtualUIView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::VIRTUAL_UI))
			continue;

		SetEntityForRendering(Entity);
		VIRTUAL_UI_SYSTEM.RenderVirtualUIComponent(Entity, CurrentCameraComponent);
	}

	entt::basic_view PointCloudView = CurrentScene->Registry.view<FEPointCloudComponent, FETransformComponent>();
	// Because we have old-style rendering and compute shader based rendering, we need to render point clouds in a different way.
	// Sorting is done in a non optimal way, but it is not a big deal for now.
	std::vector<FEEntity*> ComputeShaderPointClouds;
	for (auto [EnTTEntity, PointCloudComponent, TransformComponent] : PointCloudView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::POINT_CLOUD))
			continue;

		if (PointCloudComponent.GetPointCloud() == nullptr)
			continue;

		SetEntityForRendering(Entity);
		if (!PointCloudComponent.GetPointCloud()->IsAdvancedRenderingEnabled())
		{
			POINT_CLOUD_SYSTEM.RenderStandard(Entity, MainCameraEntity);
			continue;
		}

		ComputeShaderPointClouds.push_back(Entity);
	}

	CurrentCameraRenderingData->SceneToTextureFB->UnBind();

	// After the usual rendering is complete, we need to render point clouds using compute shaders.
	if (!ComputeShaderPointClouds.empty())
	{
		if (!CurrentCameraRenderingData->IsAdvancedPointCloudRenderingInitialized())
		{
			if (!RENDERER.InitializeComputeShaderPointCloudRendering(MainCameraEntity))
			{
				LOG.Add("Function FERenderer::Render, RENDERER.InitializeComputeShaderPointCloudRendering(MainCameraEntity) failed!", "FE_LOG_RENDERING", FE_LOG_ERROR);
			}
		}

		if (CurrentCameraRenderingData->IsAdvancedPointCloudRenderingInitialized())
		{
			int ScreenWidth = CurrentCameraComponent.GetRenderTargetWidth();
			int ScreenHeight = CurrentCameraComponent.GetRenderTargetHeight();

			for (size_t i = 0; i < ComputeShaderPointClouds.size(); i++)
			{
				FETransformComponent& TransformComponent = ComputeShaderPointClouds[i]->GetComponent<FETransformComponent>();
				POINT_CLOUD_SYSTEM.RenderWithComputeShaders(TransformComponent, ComputeShaderPointClouds[i]->GetComponent<FEPointCloudComponent>(), MainCameraEntity);
			}

			POINT_CLOUD_SYSTEM.FuseComputeRenderedToFramebuffer(MainCameraEntity);
		}
	}

	CurrentCameraRenderingData->FinalScene = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
	this->CurrentCameraRenderingData = nullptr;
}

FECameraRenderingData* FERenderer::CreateCameraRenderingData(FEEntity* CameraEntity)
{
	FECameraRenderingData* Result = nullptr;

	if (CameraEntity == nullptr)
		return Result;

	if (!CameraEntity->HasComponent<FECameraComponent>())
		return Result;

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	if (CameraComponent.GetRenderTargetWidth() <= 0 || CameraComponent.GetRenderTargetHeight() <= 0)
		return Result;

	Result = new FECameraRenderingData();
	Result->CameraEntity = CameraEntity;
	Result->SceneToTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());

	Result->SceneColorScratchTexture = RESOURCE_MANAGER.CreateSameFormatTexture(Result->SceneToTextureFB->GetColorAttachment());

	if (CameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified)
		return Result;

	Result->GBuffer = new FEGBuffer(Result->SceneToTextureFB);
	Result->SSAO = new FESSAO(Result->SceneToTextureFB);

	Result->DepthPyramid = RESOURCE_MANAGER.CreateTexture();
	RESOURCE_MANAGER.Textures.erase(Result->DepthPyramid->GetObjectID());

	Result->DepthPyramid->Bind();
	Result->DepthPyramid->SetUWrapType(FE_TEXTURE_WRAP_TYPE::CLAMP_TO_EDGE);
	Result->DepthPyramid->SetVWrapType(FE_TEXTURE_WRAP_TYPE::CLAMP_TO_EDGE);
	Result->DepthPyramid->SetFilterType(FE_TEXTURE_MINMAG_FILTER_TYPE::NEAREST);

	const int MaxDimension = std::max(CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());
	const size_t MipmapCount = static_cast<size_t>(floor(log2(MaxDimension)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipmapCount), GL_R32F, CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight()));
	Result->DepthPyramid->Width = CameraComponent.GetRenderTargetWidth();
	Result->DepthPyramid->Height = CameraComponent.GetRenderTargetHeight();

	glm::ivec2 ViewportSize = glm::ivec2(CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());
	if (CameraComponent.GetRenderScale() != 1.0f)
	{
		ViewportSize.x = CameraComponent.Viewport->GetWidth();
		ViewportSize.y = CameraComponent.Viewport->GetHeight();
	}
	
	// ************************************ Bloom ************************************
	FEPostProcess* BloomEffect = ENGINE.CreatePostProcess("Bloom", static_cast<int>(ViewportSize.x / 4.0f), static_cast<int>(ViewportSize.y / 4.0f));
	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_SCENE_HDR_COLOR, RESOURCE_MANAGER.GetShader("0C19574118676C2E5645200E"/*"FEBloomThreshold"*/)));
	BloomEffect->Stages[0]->Shader->UpdateUniformData("thresholdBrightness", 1.0f);

	FEShader* BloomBlurShader = RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 5.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 5.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 1.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, BloomBlurShader));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 1.0f));

	FEShader* BloomCompositionShader = RESOURCE_MANAGER.GetShader("1833272551376C2E5645200E"/*"FEBloomComposition"*/);
	BloomEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_HDR_COLOR}, BloomCompositionShader));

	RENDERER.AddPostProcess(Result, BloomEffect);
	// ************************************ Bloom END ************************************

	// ************************************ Gamma & HDR ************************************
	FEPostProcess* GammaHDR = ENGINE.CreatePostProcess("GammaAndHDR", ViewportSize.x, ViewportSize.y);
	FEShader* GammaHDRShader = RESOURCE_MANAGER.GetShader("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/);
	GammaHDR->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, GammaHDRShader));
	RENDERER.AddPostProcess(Result, GammaHDR);
	// ************************************ Gamma & HDR END ************************************

	// ************************************ FXAA ***************************************
	FEPostProcess* FEFXAAEffect = ENGINE.CreatePostProcess("FE_FXAA", ViewportSize.x, ViewportSize.y);
	FEShader* FEFXAAShader = RESOURCE_MANAGER.GetShader("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/);
	FEFXAAEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FEFXAAShader));
	RENDERER.AddPostProcess(Result, FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	Result->PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ViewportSize.x, ViewportSize.y));
	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FEPostProcess* DOFEffect = ENGINE.CreatePostProcess("DOF", ViewportSize.x, ViewportSize.y);
	FEShader* DOFShader = RESOURCE_MANAGER.GetShader("7800253C244442155D0F3C7B"/*"DOF"*/);
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));

	RENDERER.AddPostProcess(Result, DOFEffect);
	// ************************************ DOF END ************************************

	// ************************************ Chromatic Aberration ************************************
	FEPostProcess* ChromaticAberrationEffect = ENGINE.CreatePostProcess("chromaticAberration", ViewportSize.x, ViewportSize.y);
	FEShader* ChromaticAberrationShader = RESOURCE_MANAGER.GetShader("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/);
	ChromaticAberrationEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0 }, ChromaticAberrationShader));
	RENDERER.AddPostProcess(Result, ChromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	Result->PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ViewportSize.x, ViewportSize.y));
	// ************************************ Chromatic Aberration END ************************************

	return Result;
}

FECameraRenderingData* FERenderer::GetCameraRenderingData(FEEntity* CameraEntity)
{
	if (CameraEntity == nullptr)
		return nullptr;

	if (CameraRenderingDataMap.find(CameraEntity->GetObjectID()) != CameraRenderingDataMap.end() && CameraRenderingDataMap[CameraEntity->GetObjectID()] != nullptr)
		return CameraRenderingDataMap[CameraEntity->GetObjectID()];
	
	FECameraRenderingData* Result = CreateCameraRenderingData(CameraEntity);
	if (Result != nullptr)
	{
		CameraRenderingDataMap[CameraEntity->GetObjectID()] = Result;
		return CameraRenderingDataMap[CameraEntity->GetObjectID()];
	}
	else
	{
		return nullptr;
	}
}

void FERenderer::ForceCameraRenderingDataUpdate(FEEntity* CameraEntity)
{
	if (CameraEntity == nullptr)
		return;

	if (CameraRenderingDataMap.find(CameraEntity->GetObjectID()) != CameraRenderingDataMap.end())
	{
		delete CameraRenderingDataMap[CameraEntity->GetObjectID()];
		CameraRenderingDataMap.erase(CameraEntity->GetObjectID());
	}

	FECameraRenderingData* Result = CreateCameraRenderingData(CameraEntity);
	CameraRenderingDataMap[CameraEntity->GetObjectID()] = Result;
}

FETexture* FERenderer::GetCameraResult(FEEntity* CameraEntity)
{
	FETexture* Result = nullptr;

	if (CameraEntity == nullptr)
		return Result;

	FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(CameraEntity);
	if (CameraRenderingData == nullptr)
		return Result;

	if (CameraRenderingData->FinalScene != nullptr)
	{
		Result = CameraRenderingData->FinalScene;
	}
	else if (CameraRenderingData->SceneToTextureFB->GetColorAttachment() != nullptr)
	{
		Result = CameraRenderingData->SceneToTextureFB->GetColorAttachment();
	}

	return Result;
}

void FERenderer::RenderInternal(FEScene* CurrentScene, FEEntity* MainCameraEntity, FECameraRenderingData* CurrentCameraRenderingData)
{
	this->CurrentCameraRenderingData = CurrentCameraRenderingData;
	UpdateShadersForCamera(CurrentCameraRenderingData);

	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();

	CurrentCameraComponent.UpdateFrustum();

	LastTestTime = TestTime;
	TestTime = 0.0f;

	// there is only 1 directional light, sun.
	// and we need to set correct light position
	//#fix it should update view matrices for each cascade!
	FEEntity* DirectionalLightEntity = nullptr;
	std::vector< std::string> LightsIDList = CurrentScene->GetEntityIDListWithComponent<FELightComponent>();
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

		DirectionalLightEntity = LightEntity;
		break;
	}

	LoadUniformBlocks(CurrentScene);

	// ********* GENERATE SHADOW MAPS *********
	const bool bPreviousState = bUseOcclusionCulling;
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
	entt::basic_view PrefabInstancedView = CurrentScene->Registry.view<FEInstancedComponent, FEPrefabInstanceComponent>();
	entt::basic_view TerrainView = CurrentScene->Registry.view<FETerrainComponent, FETransformComponent>();
	entt::basic_view VirtualUIView = CurrentScene->Registry.view<FEVirtualUIComponent, FETransformComponent>();
	entt::basic_view PointCloudView = CurrentScene->Registry.view<FEPointCloudComponent, FETransformComponent>();
	entt::basic_view VolumeView = CurrentScene->Registry.view<FEVolumeComponent, FETransformComponent>();

	for (std::string EntityID : LightsIDList)
	{
		FEEntity* LightEntity = CurrentScene->GetEntity(EntityID);
		FETransformComponent& TransformComponent = LightEntity->GetComponent<FETransformComponent>();
		FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

		if (LightComponent.GetType() != FE_DIRECTIONAL_LIGHT)
			continue;

		if (LightComponent.IsCastShadows())
		{
			const float ShadowsBlurFactor = LightComponent.GetShadowBlurFactor();
			ShaderPBR->UpdateUniformData("shadowBlurFactor", ShadowsBlurFactor);
			ShaderInstancedPBR->UpdateUniformData("shadowBlurFactor", ShadowsBlurFactor);

			const glm::vec3 OldCameraPosition = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE);
			const glm::mat4 OldViewMatrix = CurrentCameraComponent.GetViewMatrix();
			const glm::mat4 OldProjectionMatrix = CurrentCameraComponent.GetProjectionMatrix();

			for (size_t i = 0; i < static_cast<size_t>(LightComponent.ActiveCascades); i++)
			{
				if (LightComponent.CascadeData[i].FrameBuffer == nullptr)
				{
					LOG.Add("Function FERenderer::Render, LightComponent.CascadeData[i].FrameBuffer is nullptr!", "FE_LOG_RENDERING", FE_LOG_ERROR);
					continue;
				}

				// Put camera to the position of light.
				CurrentCameraComponent.ProjectionMatrix = LightComponent.CascadeData[i].ProjectionMat;
				CurrentCameraComponent.ViewMatrix = LightComponent.CascadeData[i].ViewMat;

				SetGLViewport(0, 0, LightComponent.CascadeData[i].FrameBuffer->GetWidth(), LightComponent.CascadeData[i].FrameBuffer->GetHeight());

				UpdateGPUCullingFrustum(MainCameraEntity);

				LightComponent.CascadeData[i].FrameBuffer->Bind();
				FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));

				for (auto [EnTTEntity, TerrainComponent, TransformComponent] : TerrainView.each())
				{
					FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
					if (Entity == nullptr)
						continue;

					if (!Entity->IsVisible())
						continue;

					if (!TerrainComponent.IsCastingShadows() || !Entity->IsComponentVisible(ComponentVisibilityType::TERRAIN))
						continue;

					TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
					SetEntityForRendering(Entity);
					RenderTerrainComponent(Entity, MainCameraEntity);
					TerrainComponent.Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
				}

				for (entt::entity EnTTEntity : GameModelGroup)
				{
					auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

					FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
					if (Entity == nullptr)
						continue;

					if (!GameModelComponent.IsCastShadows() || !Entity->IsComponentVisible(ComponentVisibilityType::GAME_MODEL))
						continue;

					if (GameModelComponent.GetGameModel() == nullptr)
						continue;
					FEMaterial* OriginalMaterial = GameModelComponent.GetGameModel()->Material;
					if (OriginalMaterial == nullptr)
						continue;

					FEMaterial* ShadowMapMaterialToUse = !Entity->HasComponent<FEInstancedComponent>() ? ShadowMapMaterial : ShadowMapMaterialInstanced;
					GameModelComponent.GetGameModel()->Material = ShadowMapMaterialToUse;
					ShadowMapMaterialToUse->SetAlbedoMap(OriginalMaterial->GetAlbedoMap());

					if (OriginalMaterial->GetAlbedoMap(1) != nullptr)
					{
						ShadowMapMaterialToUse->SetAlbedoMap(OriginalMaterial->GetAlbedoMap(1), 1);
						ShadowMapMaterialToUse->GetAlbedoMap(1)->Bind(1);
					}

					SetEntityForRendering(Entity);
					if (!Entity->HasComponent<FEInstancedComponent>())
					{
						RenderGameModelComponent(Entity, MainCameraEntity, false);
					}
					else if (Entity->HasComponent<FEInstancedComponent>())
					{
						RenderGameModelComponentWithInstanced(Entity, MainCameraEntity, true, false);
					}

					GameModelComponent.GetGameModel()->Material = OriginalMaterial;
					for (size_t k = 0; k < ShadowMapMaterial->Textures.size(); k++)
					{
						ShadowMapMaterialToUse->Textures[k] = nullptr;
						ShadowMapMaterialToUse->TextureBindings[k] = -1;
					}
				}

				for (auto [EnTTEntity, InstancedComponent, PrefabInstanceComponent] : PrefabInstancedView.each())
				{
					FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
					FEMaterial* OriginalMaterial;

					for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
					{
						FEEntity* EntityWithGameModel = INSTANCED_RENDERING_SYSTEM.GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
						if (EntityWithGameModel == nullptr)
							continue;

						FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();

						OriginalMaterial = GameModelComponent.GetGameModel()->Material;
						FEMaterial* ShadowMapMaterialToUse = ShadowMapMaterialInstanced;
						GameModelComponent.GetGameModel()->Material = ShadowMapMaterialToUse;
						ShadowMapMaterialToUse->SetAlbedoMap(OriginalMaterial->GetAlbedoMap());

						if (OriginalMaterial->GetAlbedoMap(1) != nullptr)
						{
							ShadowMapMaterialToUse->SetAlbedoMap(OriginalMaterial->GetAlbedoMap(1), 1);
							ShadowMapMaterialToUse->GetAlbedoMap(1)->Bind(1);
						}

						SetEntityForRendering(Entity);
						RenderGameModelComponentWithInstanced(Entity, MainCameraEntity, true, false, i);

						GameModelComponent.GetGameModel()->Material = OriginalMaterial;
						for (size_t k = 0; k < ShadowMapMaterial->Textures.size(); k++)
						{
							ShadowMapMaterialToUse->Textures[k] = nullptr;
							ShadowMapMaterialToUse->TextureBindings[k] = -1;
						}
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

			SetGLViewport(0, 0, CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight());
			break;
		}
	}

	bUseOcclusionCulling = bPreviousState;
	// ********* GENERATE SHADOW MAPS END *********

	// in current version only shadows from one directional light is supported.
	if (DirectionalLightEntity != nullptr)
	{
		if (CSM0) CSM0->Bind(FE_CSM_UNIT);
		if (CSM1) CSM1->Bind(FE_CSM_UNIT + 1);
		if (CSM2) CSM2->Bind(FE_CSM_UNIT + 2);
		if (CSM3) CSM3->Bind(FE_CSM_UNIT + 3);
	}

	// ********* RENDER SCENE *********

	SetGLViewport(0, 0, CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight());
	CurrentCameraRenderingData->GBuffer->GFrameBuffer->Bind();

	const unsigned int FrameBufferColorAttachments[7] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
	FE_GL_ERROR(glDrawBuffers(7, FrameBufferColorAttachments));

	if (CurrentCameraComponent.IsClearColorEnabled())
	{
		glm::vec4 ClearColor = CurrentCameraComponent.GetClearColor();
		FE_GL_ERROR(glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	UpdateGPUCullingFrustum(MainCameraEntity);

	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::GAME_MODEL) || !GameModelComponent.IsPostprocessApplied())
			continue;

		SetEntityForRendering(Entity);
		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			ForceShader(RESOURCE_MANAGER.GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
			RenderGameModelComponent(Entity, MainCameraEntity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{

			ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
			RenderGameModelComponentWithInstanced(Entity, MainCameraEntity);
		}
	}

	for (auto [EnTTEntity, InstancedComponent, PrefabInstanceComponent] : PrefabInstancedView.each())
	{
		ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);

		SetEntityForRendering(Entity);
		for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
			RenderGameModelComponentWithInstanced(Entity, MainCameraEntity, false, false, i);
	}

	for (auto [EnTTEntity, VirtualUIComponent, TransformComponent] : VirtualUIView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;
		
		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::VIRTUAL_UI))
			continue;

		SetEntityForRendering(Entity);
		VIRTUAL_UI_SYSTEM.RenderVirtualUIComponent(Entity, CurrentCameraComponent);
	}

	for (auto [EnTTEntity, TerrainComponent, TransformComponent] : TerrainView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::TERRAIN))
			continue;

		RenderTerrainComponent(Entity, MainCameraEntity);
	}

	// Because we have old-style rendering and compute shader based rendering, we need to render point clouds in a different way.
	// Sorting is done in a non optimal way, but it is not a big deal for now.
	std::vector<FEEntity*> ComputeShaderPointClouds;
	for (auto [EnTTEntity, PointCloudComponent, TransformComponent] : PointCloudView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
		{
			std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
			for (const auto& ExistingCallback : Callbacks)
			{
				if (ExistingCallback != nullptr)
					ExistingCallback(Entity);
			}
		}

		if (!Entity->IsComponentVisible(ComponentVisibilityType::POINT_CLOUD))
			continue;

		if (PointCloudComponent.GetPointCloud() == nullptr)
			continue;

		if (!PointCloudComponent.GetPointCloud()->IsAdvancedRenderingEnabled())
		{
			POINT_CLOUD_SYSTEM.RenderStandard(Entity, MainCameraEntity);
			continue;
		}

		ComputeShaderPointClouds.push_back(Entity);
	}

	CurrentCameraRenderingData->GBuffer->GFrameBuffer->UnBind();

	// After the usual rendering is complete, we need to render point clouds using compute shaders.
	if (!ComputeShaderPointClouds.empty())
	{
		if (!CurrentCameraRenderingData->IsAdvancedPointCloudRenderingInitialized())
		{
			if (!RENDERER.InitializeComputeShaderPointCloudRendering(MainCameraEntity))
			{
				LOG.Add("Function FERenderer::Render, RENDERER.InitializeComputeShaderPointCloudRendering(MainCameraEntity) failed!", "FE_LOG_RENDERING", FE_LOG_ERROR);
			}
		}

		if (CurrentCameraRenderingData->IsAdvancedPointCloudRenderingInitialized())
		{
			int ScreenWidth = CurrentCameraComponent.GetRenderTargetWidth();
			int ScreenHeight = CurrentCameraComponent.GetRenderTargetHeight();

			for (size_t i = 0; i < ComputeShaderPointClouds.size(); i++)
			{
				FETransformComponent& TransformComponent = ComputeShaderPointClouds[i]->GetComponent<FETransformComponent>();
				POINT_CLOUD_SYSTEM.RenderWithComputeShaders(TransformComponent, ComputeShaderPointClouds[i]->GetComponent<FEPointCloudComponent>(), MainCameraEntity);
			}

			POINT_CLOUD_SYSTEM.FuseComputeRenderedToFramebuffer(MainCameraEntity);
		}
	}

	ForceShader(nullptr);

	CurrentCameraRenderingData->GBuffer->Albedo->Bind(0);
	CurrentCameraRenderingData->GBuffer->Normals->Bind(1);
	CurrentCameraRenderingData->GBuffer->MaterialProperties->Bind(2);
	CurrentCameraRenderingData->GBuffer->Positions->Bind(3);
	CurrentCameraRenderingData->GBuffer->ShaderProperties->Bind(4);

	// ************************************ SSAO ************************************
	UpdateSSAO(MainCameraEntity);
	// ************************************ SSAO END ************************************

	// ************************************ COPYING DEPTH BUFFER ************************************
	glBindFramebuffer(GL_READ_FRAMEBUFFER, CurrentCameraRenderingData->GBuffer->GFrameBuffer->FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, CurrentCameraRenderingData->SceneToTextureFB->FBO); // write to default framebuffer
	glBlitFramebuffer(0, 0, CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight(),
					  0, 0, CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight(),
					  GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	// ************************************ COPYING DEPTH BUFFER END ************************************

	CurrentCameraRenderingData->GBuffer->Albedo->Bind(0);
	CurrentCameraRenderingData->GBuffer->Normals->Bind(1);
	CurrentCameraRenderingData->GBuffer->MaterialProperties->Bind(2);
	CurrentCameraRenderingData->GBuffer->Positions->Bind(3);
	CurrentCameraRenderingData->GBuffer->ShaderProperties->Bind(4);
	CurrentCameraRenderingData->SSAO->FB->GetColorAttachment()->Bind(5);
	CurrentCameraRenderingData->GBuffer->MotionVectors->Bind(6);
	CurrentCameraRenderingData->SceneToTextureFB->Bind();

	const unsigned int ColorAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, ColorAttachments);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* FinalSceneShader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	FinalSceneShader->Start();
	FinalSceneShader->UpdateUniformData("SSAOActive", CurrentCameraComponent.IsSSAOEnabled() ? 1.0f : 0.0f);
	LoadStandardUniforms(FinalSceneShader, true, MainCameraEntity);
	FinalSceneShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	CurrentCameraRenderingData->GBuffer->Albedo->UnBind();
	CurrentCameraRenderingData->GBuffer->Normals->UnBind();
	CurrentCameraRenderingData->GBuffer->MaterialProperties->UnBind();
	CurrentCameraRenderingData->GBuffer->Positions->UnBind();
	CurrentCameraRenderingData->GBuffer->ShaderProperties->UnBind();
	CurrentCameraRenderingData->GBuffer->MotionVectors->UnBind();

	FinalSceneShader->Stop();

	glDepthMask(GL_TRUE);
	// Could impact depth pyramid construction( min vs max ).
	glDepthFunc(GL_LESS);

	RenderDebugLines(CurrentScene, MainCameraEntity, CurrentCameraRenderingData);

	// ********* RENDER SKY *********
	entt::basic_view SkyDomeView = CurrentScene->Registry.view<FESkyDomeComponent, FETransformComponent>();
	for (auto [EnTTEntity, SkyDomeComponent, TransformComponent] : SkyDomeView.each())
	{
		FEEntity* CurrentEntity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (CurrentEntity == nullptr)
			continue;

		if (!CurrentEntity->IsVisible())
			continue;

		if (!SKY_DOME_SYSTEM.IsEnabled())
		{
			CurrentEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, false);
			break;
		}

		CurrentEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, true);
		SetEntityForRendering(CurrentEntity);
		RenderGameModelComponent(CurrentEntity, MainCameraEntity);
		CurrentEntity->SetComponentVisible(ComponentVisibilityType::GAME_MODEL, false);
		// Only one sky dome is supported.
		break;
	}
	// ********* RENDER SCENE END *********
	CurrentCameraRenderingData->SceneToTextureFB->UnBind();

	//Generate the mipmaps of ColorAttachment
	CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment()->Bind();
	glGenerateMipmap(GL_TEXTURE_2D);

	// ********* VOLUMETRIC PASS *************
	// Each volume reads the previous scene color and writes the composite.
	// Reading and writing the same texture is a feedback loop, so we alternate two color textures and swap them per volume.
	bool bFirstVolumetricComponent = true;
	FETexture* OriginalSceneColor = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
	FETexture* CurrentSourceSceneColorTexture = OriginalSceneColor;
	FETexture* CurrentTargetSceneColorTexture = CurrentCameraRenderingData->SceneColorScratchTexture;
	for (auto [EnTTEntity, VolumeComponent, TransformComponent] : VolumeView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (VolumeComponent.VolumeMaterial == nullptr)
			continue;

		// FE_TO_DO: This is a temporary solution, that code is too specific to a shader.
		FEShader* CurrentShader = VolumeComponent.VolumeMaterial->GetShader();
		if (CurrentShader != nullptr && CurrentShader->HasUniform("TransferFunctionTexture")
									 && VolumeComponent.VolumeMaterial->GetTextureOverride("TransferFunctionTexture") == nullptr)
		{
			VOLUME_SYSTEM.InitializeTransferFunctionTexture(VolumeComponent.ParentEntity);
			VOLUME_SYSTEM.BakeTransferFunction(VolumeComponent.ParentEntity);
		}
		// FE_TO_DO END.

		if (!VolumeComponent.VolumeMaterial->IsAllUsedTexturesNonNullptrs())
			continue;

		// If we have any volumetric component visible.
		if (bFirstVolumetricComponent)
		{
			bFirstVolumetricComponent = false;
			// FE_TO_DO: That should be set depending on material rendering mode ?
			glDepthMask(GL_FALSE);
		}

		CurrentCameraRenderingData->SceneToTextureFB->SetColorAttachment(CurrentTargetSceneColorTexture);
		CurrentCameraRenderingData->SceneToTextureFB->Bind();
		CurrentCameraRenderingData->CurrentSceneColorSourceTexture = CurrentSourceSceneColorTexture;

		SetEntityForRendering(Entity);
		VOLUME_SYSTEM.RenderVolumeComponent(TransformComponent, VolumeComponent, MainCameraEntity);

		std::swap(CurrentSourceSceneColorTexture, CurrentTargetSceneColorTexture);
	}

	if (!bFirstVolumetricComponent)
	{
		CurrentCameraRenderingData->SceneToTextureFB->UnBind();
		glDepthMask(GL_TRUE);

		// Restore the original attachment, if the result landed in the intermediate texture (odd number of volumes), copy it back.
		CurrentCameraRenderingData->SceneToTextureFB->SetColorAttachment(OriginalSceneColor);
		if (CurrentSourceSceneColorTexture != OriginalSceneColor)
			RenderToFrameBuffer(CurrentSourceSceneColorTexture, CurrentCameraRenderingData->SceneToTextureFB);
	}

	// ********* VOLUMETRIC PASS END *********
	
	// ********* Upscale rendering result if needed *************

	// Nothing here for now.

	// ********* Upscale rendering result if needed END *********

	// ********* POST_PROCESS EFFECTS *********
	// Because we render post process effects with screen quad
	// we will turn off write to depth buffer in order to get clear DB to be able to render additional objects
	glDepthMask(GL_FALSE);

	FETexture* PreviousStageTexture = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();

	// FE_FIX_ME: Temporary hack to force HDR output.
	if (CurrentCameraRenderingData->bTemporaryForceHDROutput)
		CurrentCameraRenderingData->FinalScene = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
	for (size_t i = 0; i < CurrentCameraRenderingData->PostProcessEffects.size(); i++)
	{
		FEPostProcess& Effect = *CurrentCameraRenderingData->PostProcessEffects[i];
		for (size_t j = 0; j < Effect.Stages.size(); j++)
		{
			Effect.Stages[j]->Shader->Start();
			LoadStandardUniforms(Effect.Stages[j]->Shader, nullptr, nullptr, MainCameraEntity);
			for (size_t k = 0; k < Effect.Stages[j]->StageSpecificUniformValues.size(); k++)
			{
				FEShaderUniform* CurrentUniform = Effect.Stages[j]->Shader->GetUniform(Effect.Stages[j]->StageSpecificUniformValues[k].GetName());
				if (CurrentUniform != nullptr)
					CurrentUniform->CurrentValue = Effect.Stages[j]->StageSpecificUniformValues[k];
			}

			Effect.Stages[j]->Shader->LoadUniformsDataToGPU();

			for (size_t k = 0; k < Effect.Stages[j]->InTextureSource.size(); k++)
			{
				if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0)
				{
					Effect.Stages[j]->InTexture[k] = PreviousStageTexture;
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
				else if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_SCENE_HDR_COLOR)
				{
					Effect.Stages[j]->InTexture[k] = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
				else if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_SCENE_DEPTH)
				{
					Effect.Stages[j]->InTexture[k] = CurrentCameraRenderingData->SceneToTextureFB->GetDepthAttachment();
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
				else if (Effect.Stages[j]->InTextureSource[k] == FE_POST_PROCESS_OWN_TEXTURE)
				{
					Effect.Stages[j]->InTexture[k]->Bind(static_cast<int>(k));
				}
			}

			FETexture* OrdinaryColorAttachment = Effect.IntermediateFramebuffer->GetColorAttachment();
			Effect.IntermediateFramebuffer->SetColorAttachment(Effect.Stages[j]->OutTexture);
			if (Effect.Stages[j]->OutTexture->Width != CurrentCameraRenderingData->SceneToTextureFB->GetWidth())
			{
				SetGLViewport(0, 0, Effect.Stages[j]->OutTexture->Width, Effect.Stages[j]->OutTexture->Height);
			}
			else
			{
				SetGLViewport(0, 0, CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight());
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

			PreviousStageTexture = Effect.Stages[j]->OutTexture;
		}
	}

	// Select last active effect output as texture to further work with.
	for (int i = static_cast<int>(CurrentCameraRenderingData->PostProcessEffects.size() - 1); i >= 0; i--)
	{
		FEPostProcess& Effect = *CurrentCameraRenderingData->PostProcessEffects[i];
		if (Effect.bActive)
		{
			if (!CurrentCameraRenderingData->bTemporaryForceHDROutput)
				CurrentCameraRenderingData->FinalScene = Effect.Stages.back()->OutTexture;
			break;
		}
	}

	glDepthMask(GL_TRUE);
	// ********* SCREEN SPACE EFFECTS END *********

	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI *********
	FETexture* OriginalColorAttachment = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
	CurrentCameraRenderingData->SceneToTextureFB->SetColorAttachment(CurrentCameraRenderingData->FinalScene);
	CurrentCameraRenderingData->SceneToTextureFB->Bind();

	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->IsVisible())
			continue;

		if (!Entity->IsComponentVisible(ComponentVisibilityType::GAME_MODEL) || GameModelComponent.IsPostprocessApplied())
			continue;

		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			SetEntityForRendering(Entity);
			RenderGameModelComponent(Entity, MainCameraEntity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			SetEntityForRendering(Entity);
			RenderGameModelComponentWithInstanced(Entity, MainCameraEntity);
		}
	}

	CurrentCameraRenderingData->SceneToTextureFB->UnBind();
	CurrentCameraRenderingData->SceneToTextureFB->SetColorAttachment(OriginalColorAttachment);
	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI END *********

	// **************************** TERRAIN EDITOR TOOLS ****************************
	TERRAIN_SYSTEM.UpdateBrush(CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay);
	// **************************** TERRAIN EDITOR TOOLS END ****************************

	// **************************** DEPTH PYRAMID ****************************
#ifdef USE_OCCLUSION_CULLING

	ComputeTextureCopy->Start();
	ComputeTextureCopy->UpdateUniformData("textureSize", glm::vec2(CurrentCameraRenderingData->DepthPyramid->GetWidth(), CurrentCameraRenderingData->DepthPyramid->GetHeight()));
	ComputeTextureCopy->LoadUniformsDataToGPU();

	CurrentCameraRenderingData->SceneToTextureFB->GetDepthAttachment()->Bind(0);
	glBindImageTexture(1, CurrentCameraRenderingData->DepthPyramid->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	ComputeTextureCopy->Dispatch(static_cast<unsigned>(ceil(float(CurrentCameraRenderingData->DepthPyramid->GetWidth()) / 32.0f)), static_cast<unsigned>(ceil(float(CurrentCameraRenderingData->DepthPyramid->GetHeight()) / 32.0f)), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const size_t MipmapCount = static_cast<size_t>(floor(log2(std::max(CurrentCameraRenderingData->DepthPyramid->GetWidth(), CurrentCameraRenderingData->DepthPyramid->GetHeight()))) + 1);
	for (size_t i = 0; i < MipmapCount; i++)
	{
		const float DownScale = static_cast<float>(pow(2.0f, i));

		ComputeDepthPyramidDownSample->Start();
		ComputeDepthPyramidDownSample->UpdateUniformData("textureSize", glm::vec2(CurrentCameraRenderingData->DepthPyramid->GetWidth() / DownScale, CurrentCameraRenderingData->DepthPyramid->GetHeight() / DownScale));
		ComputeDepthPyramidDownSample->LoadUniformsDataToGPU();
		glBindImageTexture(0, CurrentCameraRenderingData->DepthPyramid->GetTextureID(), static_cast<GLint>(i), GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
		glBindImageTexture(1, CurrentCameraRenderingData->DepthPyramid->GetTextureID(), static_cast<GLint>(i + 1), GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

		ComputeDepthPyramidDownSample->Dispatch(static_cast<unsigned>(ceil(float(CurrentCameraRenderingData->DepthPyramid->GetWidth() / DownScale) / 32.0f)), static_cast<unsigned>(ceil(float(CurrentCameraRenderingData->DepthPyramid->GetHeight() / DownScale) / 32.0f)), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
#endif // USE_OCCLUSION_CULLING
	// **************************** DEPTH PYRAMID END ****************************
}

void FERenderer::RenderDebugLines(FEScene* CurrentScene, FEEntity* MainCameraEntity, FECameraRenderingData* CurrentCameraRenderingData)
{
	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, DebugLinesBuffer));
	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, FE_MAX_DEBUG_LINES * sizeof(FELine), this->DebugLines.data()));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	LINE_SYSTEM.InstancedLineShader->Start();
	LINE_SYSTEM.InstancedLineShader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	LINE_SYSTEM.InstancedLineShader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	glm::mat4 IdentityMat4(1.0f);
	LINE_SYSTEM.InstancedLineShader->UpdateUniformData("FEWorldMatrix", IdentityMat4);
	LINE_SYSTEM.InstancedLineShader->UpdateUniformData("resolution", glm::vec2(CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight()));
	LINE_SYSTEM.InstancedLineShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(DebugLinesVAO));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glEnableVertexAttribArray(1));
	FE_GL_ERROR(glEnableVertexAttribArray(2));
	FE_GL_ERROR(glEnableVertexAttribArray(3));
	FE_GL_ERROR(glEnableVertexAttribArray(4));
	FE_GL_ERROR(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, DebugLineCounter));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glDisableVertexAttribArray(1));
	FE_GL_ERROR(glDisableVertexAttribArray(2));
	FE_GL_ERROR(glDisableVertexAttribArray(3));
	FE_GL_ERROR(glDisableVertexAttribArray(4));
	FE_GL_ERROR(glBindVertexArray(0));

	LINE_SYSTEM.InstancedLineShader->Stop();
}

void FERenderer::Render(FEScene* CurrentScene)
{
	if (CurrentScene == nullptr)
		return;

	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(CurrentScene);
	if (MainCameraEntity == nullptr)
		return;

	FECameraRenderingData* CurrentCameraRenderingData = GetCameraRenderingData(MainCameraEntity);
	if (CurrentCameraRenderingData == nullptr)
		return;
	
	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
	if (!CurrentCameraComponent.IsRenderingEnabled())
		return;

	if (CurrentCameraComponent.GetRenderTargetWidth() <= 0 || CurrentCameraComponent.GetRenderTargetHeight() <= 0)
		return;

	if (CurrentCameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified)
	{
		SimplifiedRender(CurrentScene, MainCameraEntity, CurrentCameraRenderingData);
	}
	else
	{
		RenderInternal(CurrentScene, MainCameraEntity, CurrentCameraRenderingData);
	}

	std::string& MainCameraID = MainCameraEntity->GetObjectID();
	if (CameraPostRenderCallbacks.find(MainCameraID) != CameraPostRenderCallbacks.end())
	{
		for (size_t i = 0; i < CameraPostRenderCallbacks[MainCameraID].size(); i++)
		{
			if (CameraPostRenderCallbacks[MainCameraID][i] != nullptr)
				CameraPostRenderCallbacks[MainCameraID][i](MainCameraEntity, GetCameraResult(MainCameraEntity));
		}
	}

	if (CurrentCameraRenderingData->CameraEntity->GetComponent<FECameraComponent>().Viewport == ENGINE.GetDefaultViewport())
	{
		FE_GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		FEMesh* ScreenQuad = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
		FEShader* ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363");
		FETexture* FinalSceneTexture = GetCameraResult(MainCameraEntity);

		FinalSceneTexture->Bind(0);
		ScreenQuadShader->Start();
		ScreenQuadShader->LoadUniformsDataToGPU();
		
		FE_GL_ERROR(glBindVertexArray(ScreenQuad->GetVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, ScreenQuad->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		FinalSceneTexture->UnBind();
		ScreenQuadShader->Stop();
	}

	this->CurrentCameraRenderingData = nullptr;
}

void FERenderer::SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith)
{
	if (SceneToWorkWith == nullptr)
	{
		LOG.Add("Scene is nullptr in FERenderer::SaveScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(SceneToWorkWith);
	if (MainCameraEntity == nullptr)
	{
		LOG.Add("Main camera entity for given scene is nullptr in FERenderer::SaveScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	FETexture* TempTexture = CreateScreenshot(MainCameraEntity);
	if (TempTexture == nullptr)
	{
		LOG.Add("Temp texture is nullptr in FERenderer::SaveScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	RESOURCE_MANAGER.SaveFETexture(TempTexture, FileName);
	RESOURCE_MANAGER.DeleteFETexture(TempTexture);
}

FETexture* FERenderer::CreateScreenshot(FEScene* Scene)
{
	if (Scene == nullptr)
	{
		LOG.Add("Scene is nullptr in FERenderer::CreateScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return nullptr;
	}

	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(Scene);
	return CreateScreenshot(MainCameraEntity);
}

FETexture* FERenderer::CreateScreenshot(FEEntity* CameraEntity)
{
	FETexture* Result = nullptr;
	if (CameraEntity == nullptr)
	{
		LOG.Add("Camera entity is nullptr in FERenderer::CreateScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}

	FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(CameraEntity);
	if (CameraRenderingData == nullptr)
	{
		LOG.Add("Camera rendering data is nullptr in FERenderer::SaveScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}

	FETexture* CameraResult = RENDERER.GetCameraResult(CameraEntity);
	if (CameraResult == nullptr)
	{
		LOG.Add("Camera result is nullptr in FERenderer::CreateScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return Result;
	}

	unsigned char* Pixels = new unsigned char[4 * CameraResult->GetWidth() * CameraResult->GetHeight()];
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, CameraResult->GetTextureID()));
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels));

	Result = RESOURCE_MANAGER.RawDataToFETexture(Pixels, CameraResult->GetWidth(), CameraResult->GetHeight());
	delete[] Pixels;

	return Result;
}

void FERenderer::RenderGameModelComponent(FEEntity* Entity, FEEntity* Camera, bool bReloadUniformBlocks)
{
	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();

	RenderGameModelComponent(GameModelComponent, TransformComponent, Entity->ParentScene, Camera, bReloadUniformBlocks);
}

void FERenderer::RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FETransformComponent& TransformComponent, FEScene* ParentScene, FEEntity* Camera, bool bReloadUniformBlocks)
{
	if (ParentScene == nullptr || GameModelComponent.GetGameModel() == nullptr || GameModelComponent.GetGameModel()->Mesh == nullptr || GameModelComponent.GetGameModel()->Material == nullptr)
		return;

	if (GameModelComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (bReloadUniformBlocks)
		LoadUniformBlocks(ParentScene);

	FEGameModel* GameModel = GameModelComponent.GetGameModel();
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
	LoadStandardUniforms(GameModel->Material->Shader, GameModel->Material, &TransformComponent, Camera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	GameModel->Material->Shader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(GameModel->Mesh->GetVaoID()));
	if ((GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
	if ((GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

	if ((GameModel->Mesh->VertexAttributes & FE_USER_DEFINED_DATA) == FE_USER_DEFINED_DATA) FE_GL_ERROR(glEnableVertexAttribArray(8));

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

// FE_FIX_ME: Should this be implemented as a separate function, or should it only be called when the camera is in forward rendering mode?
// Note: This function is currently only used in VR rendering and Simplified mode!
void FERenderer::RenderGameModelComponentForward(FEEntity* Entity, FEEntity* Camera, bool bReloadUniformBlocks)
{
	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();

	if (bReloadUniformBlocks)
		LoadUniformBlocks(Entity->ParentScene);

	FEGameModel* GameModel = GameModelComponent.GetGameModel();
	if (GameModel == nullptr)
	{
		LOG.Add("Trying to draw FEGameModelComponent with GameModel that is nullptr in FERenderer::RenderGameModelComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	if (GameModelComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GameModel->Material->Bind();
	LoadStandardUniforms(GameModel->Material->Shader, GameModel->Material, &TransformComponent, Camera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	GameModel->Material->Shader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(GameModel->Mesh->GetVaoID()));
	if ((GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
	if ((GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

	if ((GameModel->Mesh->VertexAttributes & FE_USER_DEFINED_DATA) == FE_USER_DEFINED_DATA) FE_GL_ERROR(glEnableVertexAttribArray(8));

	if ((GameModel->Mesh->VertexAttributes & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
	if ((GameModel->Mesh->VertexAttributes & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, GameModel->Mesh->GetVertexCount()));

	FE_GL_ERROR(glBindVertexArray(0));

	GameModel->Material->UnBind();

	if (GameModelComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FERenderer::RenderTerrainComponent(FEEntity* TerrainEntity, FEEntity* Camera)
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
			if (TerrainComponent.Layers[i] != nullptr && TerrainComponent.Layers[i]->GetMaterial()->IsCompactPacking())
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
	LoadStandardUniforms(TerrainComponent.Shader, nullptr, &TransformComponent, Camera, TerrainComponent.IsReceivingShadows());
	// ************ Load materials data for all Terrain layers ************

	const int LayersUsed = TerrainComponent.LayersUsed();
	if (LayersUsed == 0)
	{
		// 0 index is for HeightMap.
		RESOURCE_MANAGER.NoTexture->Bind(1);
	}

	TerrainComponent.LoadLayersDataToGPU();
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, TerrainComponent.GPULayersDataBuffer));

	// Shadow map shader does not have this parameter.
	if (TerrainComponent.Shader->GetUniform("usedLayersCount") != nullptr)
		TerrainComponent.Shader->UpdateUniformData("usedLayersCount", static_cast<float>(LayersUsed));
	// ************ Load materials data for all Terrain layers END ************

	TerrainComponent.Shader->UpdateUniformData("HeightScale", TerrainComponent.HeightScale);
	TerrainComponent.Shader->UpdateUniformData("scaleFactor", TerrainComponent.ScaleFactor);
	if (TerrainComponent.Shader->GetName() != "FESMTerrainShader")
		TerrainComponent.Shader->UpdateUniformData("tileMult", TerrainComponent.TileMult);
	TerrainComponent.Shader->UpdateUniformData("LODlevel", TerrainComponent.LODLevel);
	TerrainComponent.Shader->UpdateUniformData("HeightMapShift", TerrainComponent.HeightMapShift);

	glm::vec3 PivotPosition = TransformComponent.GetPosition();
	TerrainComponent.ScaleFactor = 1.0f * TerrainComponent.ChunkPerSide;

	static int PVMHash = static_cast<int>(std::hash<std::string>{}("FEPVMMatrix"));
	static int WorldMatrixHash = static_cast<int>(std::hash<std::string>{}("FEWorldMatrix"));
	static int HeightMapShiftHash = static_cast<int>(std::hash<std::string>{}("HeightMapShift"));

	TerrainComponent.Shader->LoadUniformsDataToGPU();
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

			FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
			FETransformComponent& CurrentCameraTransformComponent = Camera->GetComponent<FETransformComponent>();

			TerrainComponent.Shader->UpdateUniformData("FEPVMMatrix", CurrentCameraComponent.GetProjectionMatrix() * CurrentCameraComponent.GetViewMatrix() * TransformComponent.GetWorldMatrix());
			if (TerrainComponent.Shader->GetUniform("FEWorldMatrix") != nullptr)
				TerrainComponent.Shader->UpdateUniformData("FEWorldMatrix", TransformComponent.GetWorldMatrix());
			TerrainComponent.Shader->UpdateUniformData("HeightMapShift", glm::vec2(i * -1.0f, j * -1.0f));

			TerrainComponent.Shader->LoadUniformDataToGPU("FEPVMMatrix");

			if (TerrainComponent.Shader->GetUniform("FEWorldMatrix") != nullptr)
				TerrainComponent.Shader->LoadUniformDataToGPU("FEWorldMatrix");

			TerrainComponent.Shader->LoadUniformDataToGPU("HeightMapShift");

			FE_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64));
		}
	}
	TerrainComponent.Shader->Stop();
	// Revert to old state to avoid any changes in TransformComponent.
	TransformComponent = OldState;

	if (TerrainComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FERenderer::DebugDrawLine(const glm::vec3 StartPoint, const glm::vec3 EndPoint, const glm::vec3 Color, const float Width)
{
	if (DebugLineCounter >= FE_MAX_DEBUG_LINES)
	{
		//LOG.Add("Tring to draw more than FE_MAX_DEBUG_LINES", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	DebugLines[DebugLineCounter].StartPoint = StartPoint;
	DebugLines[DebugLineCounter].EndPoint = EndPoint;
	DebugLines[DebugLineCounter].Color = Color;
	DebugLines[DebugLineCounter].Width = Width;

	DebugLineCounter++;
}

void FERenderer::DebugDrawLine(FELine LineToRender)
{
	if (DebugLineCounter >= FE_MAX_DEBUG_LINES)
	{
		//LOG.Add("Tring to draw more than FE_MAX_DEBUG_LINES", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	DebugLines[DebugLineCounter] = LineToRender;
	DebugLinesDirtyFlags[DebugLineCounter].bSeenStartFrame = false;
	DebugLinesDirtyFlags[DebugLineCounter].bSeenEndFrame = false;
	DebugLinesDirtyFlags[DebugLineCounter].bCurrentlyInUse = true;
	DebugLineCounter++;
}

void FERenderer::BeginFrameDebugLines()
{
	for (size_t i = 0; i < DebugLines.size(); i++)
	{
		if (!DebugLinesDirtyFlags[i].bCurrentlyInUse)
			continue;

		DebugLinesDirtyFlags[i].bSeenStartFrame = true;
	}
}

void FERenderer::EndFrameDebugLines()
{
	// Go through all lines and if debug dirty flag seen both start and end of frame, remove line.
	for (size_t i = 0; i < DebugLines.size(); i++)
	{
		if (!DebugLinesDirtyFlags[i].bCurrentlyInUse)
			continue;

		if (DebugLinesDirtyFlags[i].bSeenStartFrame && DebugLinesDirtyFlags[i].bSeenEndFrame)
		{
			DebugLinesDirtyFlags[i].bSeenStartFrame = false;
			DebugLinesDirtyFlags[i].bSeenEndFrame = false;
			DebugLinesDirtyFlags[i].bCurrentlyInUse = false;
			DebugLines[i] = FELine();
			
			// Also shift all lines after this one to the left by one.
			// We need to keep lines array compact for rendering.
			for (size_t j = i; j < DebugLines.size() - 1; j++)
			{
				DebugLines[j] = DebugLines[j + 1];
				DebugLinesDirtyFlags[j] = DebugLinesDirtyFlags[j + 1];
			}

			DebugLineCounter--;
			i--;
		}
		else
		{
			DebugLinesDirtyFlags[i].bSeenEndFrame = true;
		}
	}
}

void FERenderer::UpdateShadersForCamera(FECameraRenderingData* CameraData)
{
	if (CameraData == nullptr)
		return;

	FECameraComponent& CameraComponent = CameraData->CameraEntity->GetComponent<FECameraComponent>();
	if (CameraComponent.GetRenderingPipeline() != FERenderingPipeline::Deferred)
		return;

	glm::ivec2 ViewportSize = glm::ivec2(CameraComponent.GetRenderTargetWidth(), CameraComponent.GetRenderTargetHeight());
	if (CameraComponent.GetRenderScale() != 1.0f)
	{
		ViewportSize.x = CameraComponent.Viewport->GetWidth();
		ViewportSize.y = CameraComponent.Viewport->GetHeight();
	}
	
	// **************************** Bloom ********************************
	CameraData->PostProcessEffects[0]->Stages[0]->Shader->UpdateUniformData("thresholdBrightness", CameraComponent.GetBloomThreshold());

	CameraData->PostProcessEffects[0]->Stages[1]->StageSpecificUniformValues[1].SetValue(CameraComponent.GetBloomSize());
	CameraData->PostProcessEffects[0]->Stages[2]->StageSpecificUniformValues[1].SetValue(CameraComponent.GetBloomSize());
	// **************************** Bloom END ****************************

	// **************************** FXAA ********************************
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAASpanMax", CameraComponent.GetFXAASpanMax());
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAAReduceMin", CameraComponent.GetFXAAReduceMin());
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAAReduceMul", CameraComponent.GetFXAAReduceMul());
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAATextureSize", glm::vec2(1.0f / ViewportSize.x, 1.0f / ViewportSize.y));
	// **************************** FXAA END ****************************

	// **************************** Depth of Field ********************************
	CameraData->PostProcessEffects[3]->Stages[0]->Shader->UpdateUniformData("depthThreshold", CameraComponent.GetDOFNearDistance());
	CameraData->PostProcessEffects[3]->Stages[1]->Shader->UpdateUniformData("depthThreshold", CameraComponent.GetDOFNearDistance());

	CameraData->PostProcessEffects[3]->Stages[0]->Shader->UpdateUniformData("depthThresholdFar", CameraComponent.GetDOFFarDistance());
	CameraData->PostProcessEffects[3]->Stages[1]->Shader->UpdateUniformData("depthThresholdFar", CameraComponent.GetDOFFarDistance());

	CameraData->PostProcessEffects[3]->Stages[0]->Shader->UpdateUniformData("blurSize", CameraComponent.GetDOFStrength());
	CameraData->PostProcessEffects[3]->Stages[1]->Shader->UpdateUniformData("blurSize", CameraComponent.GetDOFStrength());

	CameraData->PostProcessEffects[3]->Stages[0]->Shader->UpdateUniformData("intMult", CameraComponent.GetDOFDistanceDependentStrength());
	CameraData->PostProcessEffects[3]->Stages[1]->Shader->UpdateUniformData("intMult", CameraComponent.GetDOFDistanceDependentStrength());

	CameraData->PostProcessEffects[3]->Stages[0]->Shader->UpdateUniformData("zNear", CameraComponent.GetNearPlane());
	CameraData->PostProcessEffects[3]->Stages[1]->Shader->UpdateUniformData("zNear", CameraComponent.GetNearPlane());

	CameraData->PostProcessEffects[3]->Stages[0]->Shader->UpdateUniformData("zFar", CameraComponent.GetFarPlane());
	CameraData->PostProcessEffects[3]->Stages[1]->Shader->UpdateUniformData("zFar", CameraComponent.GetFarPlane());
	// **************************** Depth of Field END ****************************

	// **************************** Chromatic Aberration ********************************
	CameraData->PostProcessEffects[4]->Stages[0]->Shader->UpdateUniformData("intensity", CameraComponent.GetChromaticAberrationIntensity());
	// **************************** Chromatic Aberration END ****************************

	// **************************** Distance Fog ********************************

	if (CameraComponent.IsDistanceFogEnabled())
	{
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("fogDensity", CameraComponent.GetDistanceFogDensity());
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("fogGradient", CameraComponent.GetDistanceFogGradient());

		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("fogDensity", CameraComponent.GetDistanceFogDensity());
		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("fogGradient", CameraComponent.GetDistanceFogGradient());
	}
	else
	{
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("fogDensity", -1.0f);
		RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->UpdateUniformData("fogGradient", -1.0f);

		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("fogDensity", -1.0f);
		RESOURCE_MANAGER.GetShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->UpdateUniformData("fogGradient", -1.0f);
	}
	// **************************** Distance Fog END ****************************

	CurrentCameraBasedEngineProvidedData.ViewMatrix = CameraComponent.GetViewMatrix();
	CurrentCameraBasedEngineProvidedData.InverseViewMatrix = glm::inverse(CurrentCameraBasedEngineProvidedData.ViewMatrix);
	CurrentCameraBasedEngineProvidedData.ProjectionMatrix = CameraComponent.GetProjectionMatrix();
	CurrentCameraBasedEngineProvidedData.InverseProjectionMatrix = glm::inverse(CurrentCameraBasedEngineProvidedData.ProjectionMatrix);
	CurrentCameraBasedEngineProvidedData.CameraPosition = CameraData->CameraEntity->GetComponent<FETransformComponent>().GetPosition(FE_WORLD_SPACE);
	CurrentCameraBasedEngineProvidedData.CameraDirection = CameraComponent.GetForward();
	CurrentCameraBasedEngineProvidedData.NearPlane = CameraComponent.GetNearPlane();
	CurrentCameraBasedEngineProvidedData.FarPlane = CameraComponent.GetFarPlane();
	CurrentCameraBasedEngineProvidedData.Gamma = CameraComponent.GetGamma();
	CurrentCameraBasedEngineProvidedData.Exposure = CameraComponent.GetExposure();
}

void FERenderer::DebugDrawAABB(FEAABB AABB, const glm::vec3 Color, const float LineWidth)
{
	std::vector<FELine> LinesToRender = GEOMETRY.GetAABBEdges(AABB);
	for (size_t i = 0; i < LinesToRender.size(); i++)
	{
		LinesToRender[i].Color = Color;
		LinesToRender[i].Width = LineWidth;
		DebugDrawLine(LinesToRender[i]);
	}
}

void FERenderer::ForceShader(FEShader* Shader)
{
	ShaderToForce = Shader;
}

void FERenderer::UpdateGPUCullingFrustum(FEEntity* Camera)
{
	float* FrustumBufferData = static_cast<float*>(glMapNamedBufferRange(FrustumInfoBuffer, 0, sizeof(float) * (32),
	                                                                     GL_MAP_WRITE_BIT |
	                                                                     GL_MAP_INVALIDATE_BUFFER_BIT |
	                                                                     GL_MAP_UNSYNCHRONIZED_BIT));

	FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = Camera->GetComponent<FETransformComponent>();

	auto Frustum = CurrentCameraComponent.GetFrustum();
	auto Coefficients = Frustum.GetAllPlanesCoefficients();

	for (size_t i = 0; i < Coefficients.size(); i++)
		FrustumBufferData[i] = Coefficients[i];
	
	FrustumBufferData[24] = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE)[0];
	FrustumBufferData[25] = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE)[1];
	FrustumBufferData[26] = CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE)[2];

	FE_GL_ERROR(glUnmapNamedBuffer(FrustumInfoBuffer));
}

void FERenderer::GPUCulling(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, FEEntity* Camera, size_t PrefabIndex)
{
	if (bFreezeCulling)
		return;

	INSTANCED_RENDERING_SYSTEM.CheckDirtyFlag(EntityWithInstancedComponent);
	GPUCullingIndividual(EntityWithInstancedComponent, GameModelComponent, Camera, PrefabIndex);
}

void FERenderer::GPUCullingIndividual(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, FEEntity* Camera, size_t BufferIndex)
{
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();

	FrustumCullingShader->Start();

	FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = Camera->GetComponent<FETransformComponent>();

	FECameraRenderingData* CurrentCameraRenderingData = GetCameraRenderingData(Camera);
#ifdef USE_OCCLUSION_CULLING
	FrustumCullingShader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	FrustumCullingShader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	FrustumCullingShader->UpdateUniformData("useOcclusionCulling", bUseOcclusionCulling);
	if (CurrentCameraRenderingData->GBuffer == nullptr)
	{
		LOG.Add("FERenderer::GPUCullingIndividual CurrentCameraRenderingData->GBuffer is nullptr", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}
	// It should be last frame size!
	const glm::vec2 RenderTargetSize = glm::vec2(CurrentCameraRenderingData->GBuffer->GFrameBuffer->DepthAttachment->GetWidth(), CurrentCameraRenderingData->GBuffer->GFrameBuffer->DepthAttachment->GetHeight());
	FrustumCullingShader->UpdateUniformData("renderTargetSize", RenderTargetSize);
	FrustumCullingShader->UpdateUniformData("nearFarPlanes", glm::vec2(CurrentCameraComponent.GetNearPlane(), CurrentCameraComponent.GetFarPlane()));
#endif // USE_OCCLUSION_CULLING

	FrustumCullingShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer));
	// TO DO: Check if frustum is updated for each camera.
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FrustumInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[0]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, InstancedComponent.InstancedElementsData[BufferIndex]->AABBSizesBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, CullingLODCountersBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, InstancedComponent.InstancedElementsData[BufferIndex]->LODInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[1]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[2]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[3]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));

	CurrentCameraRenderingData->DepthPyramid->Bind(0);
	FrustumCullingShader->Dispatch(static_cast<GLuint>(ceil(InstancedComponent.InstanceCount / 64.0f)), 1, 1);
	FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));
}

std::unordered_map<std::string, std::function<FETexture* ()>> FERenderer::GetDebugOutputTextures()
{
	return DebugOutputTextures;
}

bool FERenderer::IsOcclusionCullingEnabled()
{
	return bUseOcclusionCulling;
}

void FERenderer::SetOcclusionCullingEnabled(const bool NewValue)
{
	bUseOcclusionCulling = NewValue;
}

void FERenderer::UpdateSSAO(FEEntity* Camera)
{
	if (Camera == nullptr)
		return;

	FECameraRenderingData* CurrentCameraRenderingData = GetCameraRenderingData(Camera);
	FECameraComponent& CameraComponent = Camera->GetComponent<FECameraComponent>();
	if (!CameraComponent.IsSSAOEnabled())
		return;

	CurrentCameraRenderingData->SSAO->FB->Bind();
	if (CurrentCameraRenderingData->SSAO->Shader == nullptr)
		CurrentCameraRenderingData->SSAO->Shader = RESOURCE_MANAGER.GetShader("1037115B676E383E36345079"/*"FESSAOShader"*/);

	CurrentCameraRenderingData->SSAO->Shader->UpdateUniformData("SampleCount", CameraComponent.GetSSAOSampleCount());
	
	CurrentCameraRenderingData->SSAO->Shader->UpdateUniformData("SmallDetails", CameraComponent.IsSSAOSmallDetailsEnabled() ? 1.0f : 0.0f);
	CurrentCameraRenderingData->SSAO->Shader->UpdateUniformData("Bias", CameraComponent.GetSSAOBias());
	CurrentCameraRenderingData->SSAO->Shader->UpdateUniformData("Radius", CameraComponent.GetSSAORadius());
	CurrentCameraRenderingData->SSAO->Shader->UpdateUniformData("RadiusSmallDetails", CameraComponent.GetSSAORadiusSmallDetails());
	CurrentCameraRenderingData->SSAO->Shader->UpdateUniformData("SmallDetailsWeight", CameraComponent.GetSSAOSmallDetailsWeight());
	
	CurrentCameraRenderingData->SSAO->Shader->Start();
	LoadStandardUniforms(CurrentCameraRenderingData->SSAO->Shader, true, CurrentCameraRenderingData->CameraEntity);
	CurrentCameraRenderingData->SSAO->Shader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	CurrentCameraRenderingData->SSAO->Shader->Stop();

	if (CameraComponent.IsSSAOResultBlurred())
	{
		// First blur stage
		FEShader* BlurShader = RESOURCE_MANAGER.GetShader("0B5770660B6970800D776542"/*"FESSAOBlurShader"*/);
		BlurShader->Start();
		if (BlurShader->GetUniform("FEBlurDirection"))
			BlurShader->UpdateUniformData("FEBlurDirection", glm::vec2(0.0f, 1.0f));
		if (BlurShader->GetUniform("BlurRadius"))
			BlurShader->UpdateUniformData("BlurRadius", 1.3f);

		BlurShader->LoadUniformsDataToGPU();

		CurrentCameraRenderingData->SSAO->FB->GetColorAttachment()->Bind(0);
		CurrentCameraRenderingData->SceneToTextureFB->GetDepthAttachment()->Bind(1);
		CurrentCameraRenderingData->GBuffer->Normals->Bind(2);
		FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		// Second blur stage
		if (BlurShader->GetUniform("FEBlurDirection"))
			BlurShader->UpdateUniformData("FEBlurDirection", glm::vec2(1.0f, 0.0f));
		if (BlurShader->GetUniform("BlurRadius"))
			BlurShader->UpdateUniformData("BlurRadius", 1.3f);

		BlurShader->LoadUniformsDataToGPU();

		CurrentCameraRenderingData->SSAO->FB->GetColorAttachment()->Bind(0);
		CurrentCameraRenderingData->SceneToTextureFB->GetDepthAttachment()->Bind(1);
		CurrentCameraRenderingData->GBuffer->Normals->Bind(2);
		FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		BlurShader->Stop();

		CurrentCameraRenderingData->SSAO->FB->GetColorAttachment()->UnBind();
		CurrentCameraRenderingData->SSAO->FB->UnBind();
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
	ScreenQuadShader->LoadUniformsDataToGPU();

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

// FIX ME! That should be replaced. 
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

	/*if (FirstSource == Target || SecondSource == Target)
	{
		LOG.Add("In FERenderer::CombineFrameBuffers, Sources and Target should be different framebuffers.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}*/

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
	CurrentShader->LoadUniformsDataToGPU();

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

void FERenderer::AddCameraPostRenderCallback(std::string CameraEntityID, std::function<void(FEEntity* CameraEntity, FETexture* RenderResult)> Callback)
{
	if (CameraEntityID.empty())
	{
		LOG.Add("Attempted to call FERenderer::AddCameraPostRenderCallback with CameraEntityID set to empty string.", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	// Check if object with this ID exists, ideally it should be checked if it is camera entity.
	if (OBJECT_MANAGER.GetFEObject(CameraEntityID) == nullptr)
	{
		LOG.Add("Attempted to call FERenderer::AddCameraPostRenderCallback with CameraEntityID that is not registered.", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (Callback == nullptr)
	{
		LOG.Add("Attempted to call FERenderer::AddCameraPostRenderCallback with Callback set to nullptr.", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	CameraPostRenderCallbacks[CameraEntityID].push_back(Callback);
}

void FERenderer::SetGLViewport(int X, int Y, int Width, int Height)
{
	SetGLViewport(glm::ivec4(X, Y, Width, Height));
}

void FERenderer::SetGLViewport(glm::ivec4 ViewPortData)
{
	FE_GL_ERROR(glViewport(ViewPortData.x, ViewPortData.y, ViewPortData.z, ViewPortData.w));
}

glm::ivec4 FERenderer::GetGLViewport()
{
	glm::ivec4 Viewport;
	FE_GL_ERROR(glGetIntegerv(GL_VIEWPORT, &Viewport[0]));
	return Viewport;
}

std::vector<FELine> FERenderer::GetFrustumLines(FEEntity* Camera, glm::vec3 Color, float LineWidth)
{
	std::vector<FELine> Result;
	if (Camera == nullptr)
		return Result;

	if (!Camera->HasComponent<FECameraComponent>())
		return Result;

	FECameraComponent& CurrentCameraComponent = Camera->GetComponent<FECameraComponent>();
	// Camera could be inactive, so we should force update frustum.
	CurrentCameraComponent.UpdateFrustum();

	Result = CurrentCameraComponent.GetFrustum().GetFrustumLines(Color, LineWidth);
	return Result;
}

void FERenderer::DebugDrawFrustum(FEEntity* Camera, glm::vec3 Color, float LineWidth)
{
	std::vector<FELine> FrustumLines = GetFrustumLines(Camera, Color, LineWidth);
	for (const FELine& Line : FrustumLines)
		DebugDrawLine(Line);
}

bool FERenderer::FuseSceneRenderings(FEEntity* FirstSceneCamera, FEEntity* SecondSceneCamera, FEFramebuffer* ResultingFrameBuffer)
{
	FECameraRenderingData* FirstCameraRenderingData = GetCameraRenderingData(FirstSceneCamera);
	FECameraComponent& FirstCameraComponent = FirstSceneCamera->GetComponent<FECameraComponent>();
	FECameraRenderingData* SecondCameraRenderingData = GetCameraRenderingData(SecondSceneCamera);
	FECameraComponent& SecondCameraComponent = SecondSceneCamera->GetComponent<FECameraComponent>();

	if (FirstCameraRenderingData == nullptr || SecondCameraRenderingData == nullptr || ResultingFrameBuffer == nullptr)
	{
		LOG.Add("In FERenderer::FuseSceneRenderings, either FirstCameraRenderingData, SecondCameraRenderingData or ResultingFrameBuffer is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstCameraComponent.GetRenderingPipeline() != SecondCameraComponent.GetRenderingPipeline())
	{
		LOG.Add("In FERenderer::FuseSceneRenderings, FirstCamera and SecondCamera have different rendering pipelines.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}
	
	// Although we are using screen quad texture, in this case we need to write to a depth buffer.
	glDepthMask(GL_TRUE);
	// But we should not check depth buffer.
	glDepthFunc(GL_ALWAYS);

	// Temporary solution for debugging.
	FEShader* FESceneFusionShader = RESOURCE_MANAGER.GetShaderByName("FESceneFusionShader")[0];
	FESceneFusionShader->Start();
	FESceneFusionShader->UpdateUniformData("FirstNearPlane", FirstCameraComponent.GetNearPlane());
	FESceneFusionShader->UpdateUniformData("FirstFarPlane", FirstCameraComponent.GetFarPlane());
	FESceneFusionShader->UpdateUniformData("SecondNearPlane", SecondCameraComponent.GetNearPlane());
	FESceneFusionShader->UpdateUniformData("SecondFarPlane", SecondCameraComponent.GetFarPlane());
	FESceneFusionShader->LoadUniformsDataToGPU();

	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->Bind(0);
	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->Bind(1);

	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->Bind(2);
	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->Bind(3);

	ResultingFrameBuffer->Bind();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	FESceneFusionShader->Stop();

	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->UnBind();
	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->UnBind();

	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->UnBind();
	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->UnBind();

	ResultingFrameBuffer->UnBind();

	// Return to default value
	glDepthFunc(GL_LESS);

	return true;
}

bool FERenderer::FuseFrameBufferDataAndCameraData(FEFramebuffer* Source, FEEntity* TargetCamera,
												  float SourceNearPlane, float SourceFarPlane,
												  glm::vec3 NormalsToWrite, glm::vec4 MaterialPropertiesToWrite,
												  glm::vec4 ShaderPropertiesToWrite, glm::vec2 MotionVectorsToWrite)
{
	FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(TargetCamera);

	if (CameraRenderingData == nullptr || Source == nullptr)
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, either CameraRenderingData or Source is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (TargetCamera->HasComponent<FECameraComponent>() == false)
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, TargetCamera does not have FECameraComponent.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CameraComponent = TargetCamera->GetComponent<FECameraComponent>();
	if (CameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified)
	{
		FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(TargetCamera);
		if (CameraRenderingData == nullptr)
		{
			LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData CameraRenderingData is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
			return false;
		}

		return FuseTwoFrameBuffers(Source, SourceNearPlane, SourceFarPlane, 
								   CameraRenderingData->SceneToTextureFB, CameraComponent.GetNearPlane(),
								   CameraComponent.GetFarPlane(), CameraRenderingData->SceneToTextureFB);
	}
	else if (CameraComponent.GetRenderingPipeline() == FERenderingPipeline::Deferred)
	{
		return FuseFrameBufferDataAndCameraData(Source->GetColorAttachment(), Source->GetDepthAttachment(), TargetCamera,
												SourceNearPlane, SourceFarPlane,
												NormalsToWrite, MaterialPropertiesToWrite,
												ShaderPropertiesToWrite, MotionVectorsToWrite);
	}

	LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, Camera has unsupported rendering pipeline.", "FE_LOG_RENDERING", FE_LOG_ERROR);
	return false;
}

bool FERenderer::FuseFrameBufferDataAndCameraData(FETexture* SourceColor, FETexture* SourceDepth, FEEntity* TargetCamera,
												  float SourceNearPlane, float SourceFarPlane,
												  glm::vec3 NormalsToWrite, glm::vec4 MaterialPropertiesToWrite,
												  glm::vec4 ShaderPropertiesToWrite, glm::vec2 MotionVectorsToWrite)
{
	FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(TargetCamera);
	FECameraComponent& CameraComponent = TargetCamera->GetComponent<FECameraComponent>();

	if (CameraRenderingData == nullptr || SourceColor == nullptr || SourceDepth == nullptr)
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, either CameraRenderingData, SourceColor or SourceDepth is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (SourceNearPlane <= 0.0f || SourceFarPlane <= 0.0f)
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, SourceNearPlane or SourceFarPlane is 0.0f or less.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (SourceColor->GetWidth() != CameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->GetWidth() ||
		SourceColor->GetHeight() != CameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->GetHeight())
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, Source and Target ColorAttachment have different sizes.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	CameraRenderingData->GBuffer->GFrameBuffer->Bind();
	const unsigned int FrameBufferColorAttachments[7] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
	FE_GL_ERROR(glDrawBuffers(7, FrameBufferColorAttachments));

	// Although we are using screen quad texture, in this case we need to write to a depth buffer.
	glDepthMask(GL_TRUE);
	// But we should not check depth buffer.
	glDepthFunc(GL_ALWAYS);

	FEShader* FuseFrameBufferDataAndCameraDataDeferred = RESOURCE_MANAGER.GetShaderByName("FE_FuseFrameBufferDataAndCameraDataDeferred")[0];
	FuseFrameBufferDataAndCameraDataDeferred->Start();
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("FirstNearPlane", CameraComponent.GetNearPlane());
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("FirstFarPlane", CameraComponent.GetFarPlane());
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("SecondNearPlane", SourceNearPlane);
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("SecondFarPlane", SourceFarPlane);
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("NormalsToWrite", NormalsToWrite);
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("MaterialPropertiesToWrite", MaterialPropertiesToWrite);
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("ShaderPropertiesToWrite", ShaderPropertiesToWrite);
	FuseFrameBufferDataAndCameraDataDeferred->UpdateUniformData("MotionVectorsToWrite", MotionVectorsToWrite);
	FuseFrameBufferDataAndCameraDataDeferred->LoadUniformsDataToGPU();

	CameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->Bind(0);
	CameraRenderingData->GBuffer->Positions->Bind(1);
	CameraRenderingData->GBuffer->Normals->Bind(2);
	CameraRenderingData->GBuffer->Albedo->Bind(3);
	CameraRenderingData->GBuffer->MaterialProperties->Bind(4);
	CameraRenderingData->GBuffer->ShaderProperties->Bind(5);
	CameraRenderingData->GBuffer->MotionVectors->Bind(6);
	CameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->Bind(7);

	SourceColor->Bind(8);
	SourceDepth->Bind(9);

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	FuseFrameBufferDataAndCameraDataDeferred->Stop();

	CameraRenderingData->GBuffer->GFrameBuffer->UnBind();

	CameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->UnBind();
	CameraRenderingData->GBuffer->Positions->UnBind();
	CameraRenderingData->GBuffer->Normals->UnBind();
	CameraRenderingData->GBuffer->Albedo->UnBind();
	CameraRenderingData->GBuffer->MaterialProperties->UnBind();
	CameraRenderingData->GBuffer->ShaderProperties->UnBind();
	CameraRenderingData->GBuffer->MotionVectors->UnBind();
	CameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->UnBind();

	SourceColor->UnBind();
	SourceDepth->UnBind();

	// Return to default value
	glDepthFunc(GL_LESS);

	return true;
}

bool FERenderer::FuseTwoFrameBuffers(FEFramebuffer* FirstSource, float FirstNearPlane, float FirstFarPlane,
									 FEFramebuffer* SecondSource, float SecondNearPlane, float SecondFarPlane, FEFramebuffer* Target)
{
	if (FirstNearPlane <= 0.0f || FirstFarPlane <= 0.0f || SecondNearPlane <= 0.0f || SecondFarPlane <= 0.0f)
	{
		LOG.Add("In FERenderer::FuseTwoFrameBuffers, FirstNearPlane, FirstFarPlane, SecondNearPlane or SecondFarPlane is 0.0f or less.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource == nullptr || SecondSource == nullptr || Target == nullptr)
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, either FirstSource, SecondSource or Target is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstSource->GetWidth() != SecondSource->GetWidth() || FirstSource->GetHeight() != SecondSource->GetHeight() ||
		FirstSource->GetWidth() != Target->GetWidth() || FirstSource->GetHeight() != Target->GetHeight())
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, FirstSource, SecondSource and Target have different sizes.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	Target->Bind();

	// Although we are using screen quad texture, in this case we need to write to a depth buffer.
	glDepthMask(GL_TRUE);
	// But we should not check depth buffer.
	glDepthFunc(GL_ALWAYS);

	FEShader* FEFuseTwoFrameBuffers = RESOURCE_MANAGER.GetShaderByName("FE_FuseTwoFrameBuffers")[0];
	if (FEFuseTwoFrameBuffers == nullptr)
	{
		LOG.Add("In FERenderer::FuseFrameBufferDataAndCameraData, FEFuseTwoFrameBuffers is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}
	FEFuseTwoFrameBuffers->Start();
	FEFuseTwoFrameBuffers->UpdateUniformData("FirstNearPlane", FirstNearPlane);
	FEFuseTwoFrameBuffers->UpdateUniformData("FirstFarPlane", FirstFarPlane);
	FEFuseTwoFrameBuffers->UpdateUniformData("SecondNearPlane", SecondNearPlane);
	FEFuseTwoFrameBuffers->UpdateUniformData("SecondFarPlane", SecondFarPlane);
	
	if (FirstSource->GetColorAttachment() != nullptr)
		FirstSource->GetColorAttachment()->Bind(0);

	if (FirstSource->GetDepthAttachment() != nullptr)
		FirstSource->GetDepthAttachment()->Bind(1);

	if (SecondSource->GetColorAttachment() != nullptr)
		SecondSource->GetColorAttachment()->Bind(2);

	if (SecondSource->GetDepthAttachment() != nullptr)
	{
		SecondSource->GetDepthAttachment()->Bind(3);
		FEFuseTwoFrameBuffers->UpdateUniformData("bSecondDepthMapIsInvaild", false);
	}
	else
	{
		FEFuseTwoFrameBuffers->UpdateUniformData("bSecondDepthMapIsInvaild", true);
	}
	FEFuseTwoFrameBuffers->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	FEFuseTwoFrameBuffers->Stop();

	if (FirstSource->GetColorAttachment() != nullptr)
		FirstSource->GetColorAttachment()->UnBind();

	if (FirstSource->GetDepthAttachment() != nullptr)
		FirstSource->GetDepthAttachment()->UnBind();

	if (SecondSource->GetColorAttachment() != nullptr)
		SecondSource->GetColorAttachment()->UnBind();

	if (SecondSource->GetDepthAttachment() != nullptr)
		SecondSource->GetDepthAttachment()->UnBind();

	Target->UnBind();

	// Return to default value
	glDepthFunc(GL_LESS);
	
	return true;
}

bool FERenderer::FuseSceneRenderings(FEEntity* FirstSceneCamera, FEEntity* SecondSceneCamera, FEEntity* CameraToPutResultIn)
{
	FECameraRenderingData* FirstCameraRenderingData = GetCameraRenderingData(FirstSceneCamera);
	FECameraComponent& FirstCameraComponent = FirstSceneCamera->GetComponent<FECameraComponent>();

	FECameraRenderingData* SecondCameraRenderingData = GetCameraRenderingData(SecondSceneCamera);
	FECameraComponent& SecondCameraComponent = SecondSceneCamera->GetComponent<FECameraComponent>();

	FECameraRenderingData* CameraToPutResultInRenderingData = GetCameraRenderingData(CameraToPutResultIn);
	FECameraComponent& CameraToPutResultInComponent = CameraToPutResultIn->GetComponent<FECameraComponent>();

	if (FirstCameraRenderingData == nullptr || SecondCameraRenderingData == nullptr || CameraToPutResultInRenderingData == nullptr)
	{
		LOG.Add("In FERenderer::FuseSceneRenderings, either FirstCameraRenderingData, SecondCameraRenderingData or CameraToPutResultInRenderingData is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstCameraComponent.GetRenderingPipeline() != SecondCameraComponent.GetRenderingPipeline() ||
		FirstCameraComponent.GetRenderingPipeline() != CameraToPutResultInComponent.GetRenderingPipeline())
	{
		LOG.Add("In FERenderer::FuseSceneRenderings, FirstCamera, SecondCamera and CameraToPutResultIn have different rendering pipelines.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (FirstCameraComponent.RenderTargetWidth != SecondCameraComponent.RenderTargetWidth ||
		FirstCameraComponent.RenderTargetHeight != SecondCameraComponent.RenderTargetHeight ||
		FirstCameraComponent.RenderTargetWidth != CameraToPutResultInComponent.RenderTargetWidth ||
		FirstCameraComponent.RenderTargetHeight != CameraToPutResultInComponent.RenderTargetHeight)
	{
		LOG.Add("In FERenderer::FuseSceneRenderings, FirstCamera, SecondCamera and CameraToPutResultIn have different render target sizes.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	CameraToPutResultInRenderingData->GBuffer->GFrameBuffer->Bind();
	const unsigned int FrameBufferColorAttachments[7] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
	FE_GL_ERROR(glDrawBuffers(7, FrameBufferColorAttachments));

	// Although we are using screen quad texture, in this case we need to write to a depth buffer.
	glDepthMask(GL_TRUE);
	// But we should not check depth buffer.
	glDepthFunc(GL_ALWAYS);

	FEShader* FESceneFusionShaderDeferred = RESOURCE_MANAGER.GetShaderByName("FE_SceneFusionDeferred")[0];
	FESceneFusionShaderDeferred->Start();
	FESceneFusionShaderDeferred->UpdateUniformData("FirstNearPlane", FirstCameraComponent.GetNearPlane());
	FESceneFusionShaderDeferred->UpdateUniformData("FirstFarPlane", FirstCameraComponent.GetFarPlane());
	FESceneFusionShaderDeferred->UpdateUniformData("SecondNearPlane", SecondCameraComponent.GetNearPlane());
	FESceneFusionShaderDeferred->UpdateUniformData("SecondFarPlane", SecondCameraComponent.GetFarPlane());
	FESceneFusionShaderDeferred->LoadUniformsDataToGPU();

	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->Bind(0);
	FirstCameraRenderingData->GBuffer->Positions->Bind(1);
	FirstCameraRenderingData->GBuffer->Normals->Bind(2);
	FirstCameraRenderingData->GBuffer->Albedo->Bind(3);
	FirstCameraRenderingData->GBuffer->MaterialProperties->Bind(4);
	FirstCameraRenderingData->GBuffer->ShaderProperties->Bind(5);
	FirstCameraRenderingData->GBuffer->MotionVectors->Bind(6);
	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->Bind(7);

	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->Bind(8);
	SecondCameraRenderingData->GBuffer->Positions->Bind(9);
	SecondCameraRenderingData->GBuffer->Normals->Bind(10);
	SecondCameraRenderingData->GBuffer->Albedo->Bind(11);
	SecondCameraRenderingData->GBuffer->MaterialProperties->Bind(12);
	SecondCameraRenderingData->GBuffer->ShaderProperties->Bind(13);
	SecondCameraRenderingData->GBuffer->MotionVectors->Bind(14);
	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->Bind(15);

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	FESceneFusionShaderDeferred->Stop();

	CameraToPutResultInRenderingData->GBuffer->GFrameBuffer->UnBind();

	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->UnBind();
	FirstCameraRenderingData->GBuffer->Positions->UnBind();
	FirstCameraRenderingData->GBuffer->Normals->UnBind();;
	FirstCameraRenderingData->GBuffer->Albedo->UnBind();
	FirstCameraRenderingData->GBuffer->MaterialProperties->UnBind();
	FirstCameraRenderingData->GBuffer->ShaderProperties->UnBind();
	FirstCameraRenderingData->GBuffer->MotionVectors->UnBind();
	FirstCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->UnBind();

	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetColorAttachment()->UnBind();
	SecondCameraRenderingData->GBuffer->Positions->UnBind();;
	SecondCameraRenderingData->GBuffer->Normals->UnBind();;
	SecondCameraRenderingData->GBuffer->Albedo->UnBind();
	SecondCameraRenderingData->GBuffer->MaterialProperties->UnBind();
	SecondCameraRenderingData->GBuffer->ShaderProperties->UnBind();
	SecondCameraRenderingData->GBuffer->MotionVectors->UnBind();
	SecondCameraRenderingData->GBuffer->GFrameBuffer->GetDepthAttachment()->UnBind();

	// Return to default value
	glDepthFunc(GL_LESS);

	return true;
}

bool FERenderer::InitializeComputeShaderPointCloudRendering(FEEntity* CameraEntity)
{
	if (CameraEntity == nullptr)
	{
		LOG.Add("In FERenderer::InitializeComputeShaderPointCloudRendering, CameraEntity is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (!CameraEntity->HasComponent<FECameraComponent>())
	{
		LOG.Add("In FERenderer::InitializeComputeShaderPointCloudRendering, CameraEntity does not have FECameraComponent.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(CameraEntity);
	if (CameraRenderingData == nullptr)
	{
		LOG.Add("In FERenderer::InitializeComputeShaderPointCloudRendering, CameraRenderingData is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (CameraRenderingData->IsAdvancedPointCloudRenderingInitialized())
	{
		LOG.Add("In FERenderer::InitializeComputeShaderPointCloudRendering, PointCloud64bitFrameBuffer and PointCloudIntermediateFrameBuffer are already initialized.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	int RenderTargetWidth = CameraComponent.GetRenderTargetWidth();
	int RenderTargetHeight = CameraComponent.GetRenderTargetHeight();

	// Create your data first
	uint64_t* InitialData = new uint64_t[RenderTargetWidth * RenderTargetHeight];

	float FarPlane = CameraComponent.GetFarPlane();
	uint32_t FarPlaneBits = *reinterpret_cast<uint32_t*>(&FarPlane);
	uint32_t PackedColorBits = (0u << 0) | (0u << 8) | (0u << 16) | (0xFF << 24);
	uint64_t PixelDepthAndColor = ((uint64_t)FarPlaneBits << 32) | PackedColorBits;

	for (int i = 0; i < RenderTargetWidth * RenderTargetHeight; i++)
	{
		InitialData[i] = PixelDepthAndColor;
	}

	FE_GL_ERROR(glGenBuffers(1, &CameraRenderingData->PointCloud64bitFrameBuffer));
	FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, CameraRenderingData->PointCloud64bitFrameBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CameraRenderingData->PointCloud64bitFrameBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, RenderTargetWidth * RenderTargetHeight * 8, InitialData, GL_DYNAMIC_DRAW));
	
	CameraRenderingData->PointCloudIntermediateFrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, RenderTargetWidth, RenderTargetHeight);

	FETexture* IntermediateDepthBuffer = RESOURCE_MANAGER.CreateTexture(GL_R32F, GL_RED, RenderTargetWidth, RenderTargetHeight, true, "IntermediateDepthBuffer");
	if (IntermediateDepthBuffer == nullptr)
	{
		LOG.Add("In FERenderer::InitializeComputeShaderPointCloudRendering, IntermediateDepthBuffer is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}
	IntermediateDepthBuffer->Bind();
	IntermediateDepthBuffer->SetUWrapType(FE_TEXTURE_WRAP_TYPE::CLAMP_TO_EDGE);
	IntermediateDepthBuffer->SetVWrapType(FE_TEXTURE_WRAP_TYPE::CLAMP_TO_EDGE);
	IntermediateDepthBuffer->SetFilterType(FE_TEXTURE_MINMAG_FILTER_TYPE::NEAREST);
	IntermediateDepthBuffer->UnBind();

	CameraRenderingData->PointCloudIntermediateFrameBuffer->SetDepthAttachment(IntermediateDepthBuffer);

	return true;
}

void FERenderer::AddBeforeRenderCallback(FEEntity* Entity, std::function<void(FEEntity*)> Callback)
{
	if (Entity == nullptr)
		return;

	if (BeforeRenderCallbacks.find(Entity->GetObjectID()) != BeforeRenderCallbacks.end())
	{
		std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
		/*for (const auto& ExistingCallback : Callbacks)
		{
			if (ExistingCallback.target<void(FEEntity*)>() == Callback.target<void(FEEntity*)>())
			{
				LOG.Add("FEInstancedSystem::AddBeforeRenderCallback: Callback already exists for entity " + Entity->GetObjectID(), "FE_LOG_ECS", FE_LOG_WARNING);
				return;
			}
		}*/
	}

	BeforeRenderCallbacks[Entity->GetObjectID()].push_back(Callback);
}

void FERenderer::RemoveBeforeRenderCallback(FEEntity* Entity, std::function<void(FEEntity*)> Callback)
{
	if (Entity == nullptr)
		return;

	if (BeforeRenderCallbacks.find(Entity->GetObjectID()) == BeforeRenderCallbacks.end())
		return;

	std::vector<std::function<void(FEEntity*)>>& Callbacks = BeforeRenderCallbacks[Entity->GetObjectID()];
	for (auto CallbackIterator = Callbacks.begin(); CallbackIterator != Callbacks.end(); ++CallbackIterator)
	{
		if (CallbackIterator->target<void(FEEntity*)>() == Callback.target<void(FEEntity*)>())
		{
			Callbacks.erase(CallbackIterator);
			return;
		}
	}
}

const std::unordered_set<std::string>& FERenderer::GetEngineProvidedUniformNames()
{
	static const std::unordered_set<std::string> Names = {
		"FEWorldMatrix",
		"FEViewMatrix",
		"FEInverseViewMatrix",
		"FEProjectionMatrix",
		"FEInverseProjectionMatrix",
		"FEPVMMatrix",
		"FEPreviousFrameViewMatrix",
		"FECameraPosition",
		"FECameraDirection",
		"FENearPlane",
		"FEFarPlane",
		"FEScreenSize",
		"FEGamma",
		"FEExposure",
		"FEReceiveShadows",
		"FEUniformLighting",

		// Textures
		"FESceneColor",
		"FESceneDepthMap",
	};

	return Names;
}

bool FERenderer::IsEngineProvidedUniform(const std::string& UniformName)
{
	const auto& Names = GetEngineProvidedUniformNames();
	return Names.find(UniformName) != Names.end();
}

const EntityBasedEngineProvidedData& FERenderer::GetCurrentEntityBasedEngineProvidedData() const
{
	return CurrentEntityBasedEngineProvidedData;
}

const CameraBasedEngineProvidedData& FERenderer::GetCurrentCameraBasedEngineProvidedData() const
{
	return CurrentCameraBasedEngineProvidedData;
}

void FERenderer::SetEntityForRendering(FEEntity* Entity)
{
	if (Entity == nullptr)
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	CurrentEntityBasedEngineProvidedData.WorldMatrix = TransformComponent.GetWorldMatrix();
}

bool FERenderer::BindEngineProvidedTexture(FEShader* Shader, const std::string& UniformName)
{
	if (Shader == nullptr)
	{
		LOG.Add("In FERenderer::BindEngineProvidedTexture, Shader is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (this->CurrentCameraRenderingData == nullptr)
	{
		LOG.Add("In FERenderer::BindEngineProvidedTexture, CurrentCameraRenderingData is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (UniformName == "FESceneColor")
	{
		FEShaderUniform* Uniform = Shader->GetUniform(UniformName);
		if (Uniform == nullptr)
		{
			LOG.Add("In FERenderer::BindEngineProvidedTexture, Uniform " + UniformName + " not found in shader " + Shader->GetName(), "FE_LOG_RENDERING", FE_LOG_ERROR);
			return false;
		}

		FETexture* SceneColorTexture = (CurrentCameraRenderingData->CurrentSceneColorSourceTexture != nullptr)
										? CurrentCameraRenderingData->CurrentSceneColorSourceTexture
										: CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment(0);

		if (SceneColorTexture == nullptr)
		{
			LOG.Add("In FERenderer::BindEngineProvidedTexture, SceneColorTexture is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
			return false;
		}

		int TextureUnit = Uniform->GetValue<int>();
		SceneColorTexture->Bind(TextureUnit);
		return true;
	}
	else if (UniformName == "FESceneDepthMap")
	{
		FEShaderUniform* Uniform = Shader->GetUniform(UniformName);
		if (Uniform == nullptr)
		{
			LOG.Add("In FERenderer::BindEngineProvidedTexture, Uniform " + UniformName + " not found in shader " + Shader->GetName(), "FE_LOG_RENDERING", FE_LOG_ERROR);
			return false;
		}

		FETexture* SceneDepthTexture = CurrentCameraRenderingData->SceneToTextureFB->GetDepthAttachment();
		if (SceneDepthTexture == nullptr)
		{
			LOG.Add("In FERenderer::BindEngineProvidedTexture, SceneDepthTexture is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
			return false;
		}

		int TextureUnit = Uniform->GetValue<int>();
		SceneDepthTexture->Bind(TextureUnit);
		return true;
	}

	return false;
}

void FEGBuffer::InitializeResources(FEFramebuffer* MainFrameBuffer)
{
	GFrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());

	Positions = RESOURCE_MANAGER.CreateTexture(GL_RGB32F, GL_RGB, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(Positions, 1);

	Normals = RESOURCE_MANAGER.CreateTexture(GL_RGB16F, GL_RGB, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(Normals, 2);

	Albedo = RESOURCE_MANAGER.CreateTexture(GL_RGBA, GL_RGBA, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(Albedo, 3);

	MaterialProperties = RESOURCE_MANAGER.CreateTexture(GL_RGBA16F, GL_RGBA, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(MaterialProperties, 4);

	ShaderProperties = RESOURCE_MANAGER.CreateTexture(GL_RGBA, GL_RGBA, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(ShaderProperties, 5);

	MotionVectors = RESOURCE_MANAGER.CreateTexture(GL_RG16F, GL_RG, MainFrameBuffer->GetColorAttachment()->GetWidth(), MainFrameBuffer->GetColorAttachment()->GetHeight());
	GFrameBuffer->SetColorAttachment(MotionVectors, 6);
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

bool FECameraRenderingData::IsAdvancedPointCloudRenderingInitialized()
{
	return PointCloud64bitFrameBuffer != GLuint(-1) && PointCloudIntermediateFrameBuffer != nullptr;
}