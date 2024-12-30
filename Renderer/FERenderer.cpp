#include "FERenderer.h"
#include "../FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetRenderer()
{
	return FERenderer::GetInstancePointer();
}
#endif

FERenderer::FERenderer()
{
}

void FERenderer::Init()
{
	RENDERER.InstancedLineShader = RESOURCE_MANAGER.CreateShader("instancedLine", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_VS.glsl").c_str()).c_str(),
																				  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//InstancedLineMaterial//FE_InstancedLine_FS.glsl").c_str()).c_str(),
																				  nullptr, nullptr, nullptr, nullptr,
																				  "7E0826291010377D564F6115");
	RESOURCE_MANAGER.SetTagIternal(RENDERER.InstancedLineShader, ENGINE_RESOURCE_TAG);

	FEShader* FEScreenQuadShader = RESOURCE_MANAGER.CreateShader("FEScreenQuadShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
																					   RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_FS.glsl").c_str()).c_str(),
																					   nullptr, nullptr, nullptr, nullptr,
																					   "7933272551311F3A1A5B2363");

	RESOURCE_MANAGER.SetTagIternal(FEScreenQuadShader, ENGINE_RESOURCE_TAG);

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

	ComputeDepthPyramidDownSample->UpdateUniformData("scaleDownBy", 2);

	FEPostProcess::ScreenQuad = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
	FEPostProcess::ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);

	FEShader* BloomThresholdShader = RESOURCE_MANAGER.CreateShader("FEBloomThreshold", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str()).c_str(),
																						RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomThreshold_FS.glsl").c_str()).c_str(),
																						nullptr, nullptr, nullptr, nullptr,
																						"0C19574118676C2E5645200E");
	RESOURCE_MANAGER.SetTagIternal(BloomThresholdShader, ENGINE_RESOURCE_TAG);

	FEShader* BloomBlurShader = RESOURCE_MANAGER.CreateShader("FEBloomBlur", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str()).c_str(),
																				RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomBlur_FS.glsl").c_str()).c_str(),
																				nullptr, nullptr, nullptr, nullptr,
																				"7F3E4F5C130B537F0846274F");
	RESOURCE_MANAGER.SetTagIternal(BloomBlurShader, ENGINE_RESOURCE_TAG);

	FEShader* BloomCompositionShader = RESOURCE_MANAGER.CreateShader("FEBloomComposition", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_Bloom_VS.glsl").c_str()).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_Bloom//FE_BloomComposition_FS.glsl").c_str()).c_str(),
																							nullptr, nullptr, nullptr, nullptr,
																							"1833272551376C2E5645200E");
	RESOURCE_MANAGER.SetTagIternal(BloomCompositionShader, ENGINE_RESOURCE_TAG);

	FEShader* GammaHDRShader = RESOURCE_MANAGER.CreateShader("FEGammaAndHDRShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_VS.glsl").c_str()).c_str(),
																					RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_GammaAndHDRCorrection//FE_Gamma_and_HDR_Correction_FS.glsl").c_str()).c_str(),
																					nullptr, nullptr, nullptr, nullptr,
																					"3417497A5E0C0C2A07456E44");
	RESOURCE_MANAGER.SetTagIternal(GammaHDRShader, ENGINE_RESOURCE_TAG);

	FEShader* FEFXAAShader = RESOURCE_MANAGER.CreateShader("FEFXAAShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_VS.glsl").c_str()).c_str(),
																			RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_FXAA//FE_FXAA_FS.glsl").c_str()).c_str(),
																			nullptr, nullptr, nullptr, nullptr,
																			"1E69744A10604C2A1221426B");
	RESOURCE_MANAGER.SetTagIternal(FEFXAAShader, ENGINE_RESOURCE_TAG);

	FEShader* DOFShader = RESOURCE_MANAGER.CreateShader("DOF", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_VS.glsl").c_str()).c_str(),
																RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_DOF//FE_DOF_FS.glsl").c_str()).c_str(),
																nullptr, nullptr, nullptr, nullptr,
																"7800253C244442155D0F3C7B");
	RESOURCE_MANAGER.SetTagIternal(DOFShader, ENGINE_RESOURCE_TAG);

	FEShader* ChromaticAberrationShader = RESOURCE_MANAGER.CreateShader("chromaticAberrationShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_VS.glsl").c_str()).c_str(),
																										RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ChromaticAberration//FE_ChromaticAberration_FS.glsl").c_str()).c_str(),
																										nullptr, nullptr, nullptr, nullptr,
																										"9A41665B5E2B05321A332D09");
	RESOURCE_MANAGER.SetTagIternal(ChromaticAberrationShader, ENGINE_RESOURCE_TAG);

	FEShader* FESSAOShader = RESOURCE_MANAGER.CreateShader("FESSAOShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_VS.glsl").c_str()).c_str(),
																			RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_FS.glsl").c_str()).c_str(),
																			nullptr, nullptr, nullptr, nullptr,
																			"1037115B676E383E36345079");

	RESOURCE_MANAGER.SetTagIternal(FESSAOShader, ENGINE_RESOURCE_TAG);

	FEShader* FESSAOBlurShader = RESOURCE_MANAGER.CreateShader("FESSAOBlurShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_ScreenQuad_VS.glsl").c_str()).c_str(),
																					RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//PostProcessEffects//FE_SSAO//FE_SSAO_Blur_FS.glsl").c_str()).c_str(),
																					nullptr, nullptr, nullptr, nullptr,
																					"0B5770660B6970800D776542");
	RESOURCE_MANAGER.SetTagIternal(FESSAOBlurShader, ENGINE_RESOURCE_TAG);

	RENDERER.ShadowMapMaterial = RESOURCE_MANAGER.CreateMaterial("shadowMapMaterial", "7C41565B2E2B05321A182D89" /*"FEShadowMapShader"*/);
	RENDERER.ShadowMapMaterial->Shader = RESOURCE_MANAGER.CreateShader("FEShadowMapShader", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_VS.glsl").c_str()).c_str(),
																							RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str()).c_str());

	RESOURCE_MANAGER.SetTagIternal(RENDERER.ShadowMapMaterial->Shader, ENGINE_RESOURCE_TAG);
	RESOURCE_MANAGER.SetTagIternal(RENDERER.ShadowMapMaterial, ENGINE_RESOURCE_TAG);

	RENDERER.ShadowMapMaterialInstanced = RESOURCE_MANAGER.CreateMaterial("shadowMapMaterialInstanced", "5634765B2E2A05321A182D1A"/*"FEShadowMapShaderInstanced"*/);
	RENDERER.ShadowMapMaterialInstanced->Shader = RESOURCE_MANAGER.CreateShader("FEShadowMapShaderInstanced", RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_INSTANCED_VS.glsl").c_str()).c_str(),
																											  RESOURCE_MANAGER.LoadGLSL((RESOURCE_MANAGER.EngineFolder + "CoreExtensions//StandardMaterial//ShadowMapMaterial//FE_ShadowMap_FS.glsl").c_str()).c_str());

	RESOURCE_MANAGER.SetTagIternal(RENDERER.ShadowMapMaterialInstanced->Shader, ENGINE_RESOURCE_TAG);
	RESOURCE_MANAGER.SetTagIternal(RENDERER.ShadowMapMaterialInstanced, ENGINE_RESOURCE_TAG);
}

void FERenderer::LoadStandardUniforms(FEShader* Shader, FEMaterial* Material, FETransformComponent* Transform, FEEntity* ForceCamera, const bool IsReceivingShadows, const bool IsUniformLighting)
{
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

	// TODO: Maybe it should be removed from here, because material->bind() should handle it.
	if (Shader->GetUniform("textureBindings") != nullptr)
		Shader->GetUniform("textureBindings")->SetValue<std::vector<int>>(Material->TextureBindings);

	// TODO: Maybe it should be removed from here, because material->bind() should handle it.
	if (Shader->GetUniform("textureChannels") != nullptr)
		Shader->GetUniform("textureChannels")->SetValue<std::vector<int>>(Material->TextureChannels);

	if (Shader->GetUniform("FEWorldMatrix") != nullptr)
		Shader->UpdateUniformData("FEWorldMatrix", Transform->GetWorldMatrix());

	if (Shader->GetUniform("FEViewMatrix") != nullptr)
		Shader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());

	if (Shader->GetUniform("FEProjectionMatrix") != nullptr)
		Shader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());

	if (Shader->GetUniform("FEPVMMatrix") != nullptr)
		Shader->UpdateUniformData("FEPVMMatrix", CurrentCameraComponent.GetProjectionMatrix() * CurrentCameraComponent.GetViewMatrix() * Transform->GetWorldMatrix());

	if (Shader->GetUniform("FECameraPosition") != nullptr)
		Shader->UpdateUniformData("FECameraPosition", CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE));

	if (Shader->GetUniform("FEGamma") != nullptr)
		Shader->UpdateUniformData("FEGamma", CurrentCameraComponent.GetGamma());

	if (Shader->GetUniform("FEExposure") != nullptr)
		Shader->UpdateUniformData("FEExposure", CurrentCameraComponent.GetExposure());

	if (Shader->GetUniform("FEReceiveShadows") != nullptr)
		Shader->UpdateUniformData("FEReceiveShadows", IsReceivingShadows);

	if (Shader->GetUniform("FEUniformLighting") != nullptr)
		Shader->UpdateUniformData("FEUniformLighting", IsUniformLighting);

	// TODO: Maybe it should be removed from here, because material->bind() should handle it.
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
			Shader->UpdateUniformData("compactMaterialPacking", Material->IsCompackPacking());
	}
}

void FERenderer::LoadStandardUniforms(FEShader* Shader, const bool IsReceivingShadows, FEEntity* ForceCamera, const bool IsUniformLighting)
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

	if (Shader->GetUniform("FEViewMatrix") != nullptr)
		Shader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());

	if (Shader->GetUniform("FEProjectionMatrix") != nullptr)
		Shader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());

	if (Shader->GetUniform("FECameraPosition") != nullptr)
		Shader->UpdateUniformData("FECameraPosition", CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE));

	if (Shader->GetUniform("FEGamma") != nullptr)
		Shader->UpdateUniformData("FEGamma", CurrentCameraComponent.GetGamma());

	if (Shader->GetUniform("FEExposure") != nullptr)
		Shader->UpdateUniformData("FEExposure", CurrentCameraComponent.GetExposure());

	if (Shader->GetUniform("FEReceiveShadows") != nullptr)
		Shader->UpdateUniformData("FEReceiveShadows", IsReceivingShadows);

	if (Shader->GetUniform("FEUniformLighting") != nullptr)
		Shader->UpdateUniformData("FEUniformLighting", IsUniformLighting);
}

void FERenderer::AddPostProcess(FECameraRenderingData* CameraRenderingData, FEPostProcess* NewPostProcess, const bool NoProcessing)
{
	if (CameraRenderingData == nullptr)
		return;

	CameraRenderingData->PostProcessEffects.push_back(NewPostProcess);

	if (NoProcessing)
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

void FERenderer::RenderGameModelComponentWithInstanced(FEEntity* Entity, FEEntity* ForceCamera, bool bShadowMap, bool bReloadUniformBlocks, size_t PrefabIndex)
{
	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	FEEntity* EntityWithGameModel = INSTANCED_RENDERING_SYSTEM.GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[PrefabIndex]->EntityIDWithGameModelComponent);
	if (EntityWithGameModel == nullptr)
		return;

	FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();

	if (bReloadUniformBlocks)
		LoadUniformBlocks(Entity->ParentScene);

	GPUCulling(Entity, GameModelComponent, PrefabIndex);

	FEGameModel* CurrentGameModel = GameModelComponent.GetGameModel();
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
	LoadStandardUniforms(CurrentGameModel->GetMaterial()->Shader, CurrentGameModel->Material, &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	CurrentGameModel->GetMaterial()->Shader->LoadUniformsDataToGPU();

	INSTANCED_RENDERING_SYSTEM.Render(Entity, GameModelComponent, PrefabIndex);

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
		LoadStandardUniforms(CurrentGameModel->GetBillboardMaterial()->Shader, CurrentGameModel->GetBillboardMaterial(), &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
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

void FERenderer::SimplifiedRender(FEScene* CurrentScene)
{
	if (CurrentScene == nullptr)
		return;

	if (CurrentScene == nullptr)
		return;

	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(CurrentScene);
	if (MainCameraEntity == nullptr)
		return;

	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();
	CurrentCameraComponent.UpdateFrustumPlanes();

	CurrentCameraRenderingData->SceneToTextureFB->Bind();

	if (CurrentCameraComponent.IsClearColorEnabled())
	{
		glm::vec4 ClearColor = CurrentCameraComponent.GetClearColor();
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	entt::basic_group GameModelGroup = CurrentScene->Registry.group<FEGameModelComponent>(entt::get<FETransformComponent>);
	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		if (!GameModelComponent.IsVisible() /*|| !GameModelComponent.IsPostprocessApplied()*/)
			continue;

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			RenderGameModelComponentForward(Entity);
		}
		// No instanced rendering for now.
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			//ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
			//RenderGameModelComponentWithInstanced(Entity);
		}
	}

	CurrentCameraRenderingData->SceneToTextureFB->UnBind();
	CurrentCameraRenderingData->FinalScene = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
	CurrentCameraRenderingData->FinalScene->Bind();

	// ********* RENDER FRAME BUFFER TO SCREEN *********
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* ScreenQuadShader = RESOURCE_MANAGER.GetShader("7933272551311F3A1A5B2363"/*"FEScreenQuadShader"*/);
	ScreenQuadShader->Start();
	// FIX ME! CAMERA
	LoadStandardUniforms(ScreenQuadShader, true);
	ScreenQuadShader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, nullptr));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	ScreenQuadShader->Stop();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	// ********* RENDER FRAME BUFFER TO SCREEN END *********

	CurrentCameraRenderingData->FinalScene->UnBind();
}

FECameraRenderingData* FERenderer::CreateCameraRenderingData(FEEntity* CameraEntity)
{
	FECameraRenderingData* Result = nullptr;

	if (CameraEntity == nullptr)
		return Result;

	if (!CameraEntity->HasComponent<FECameraComponent>())
		return Result;

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	if (CameraComponent.RenderTargetWidth <= 0 || CameraComponent.RenderTargetHeight <= 0)
		return Result;

	Result = new FECameraRenderingData();
	Result->CameraEntity = CameraEntity;
	Result->SceneToTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);

	if (CameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified)
		return Result;

	Result->GBuffer = new FEGBuffer(Result->SceneToTextureFB);
	Result->SSAO = new FESSAO(Result->SceneToTextureFB);

	Result->DepthPyramid = RESOURCE_MANAGER.CreateTexture();
	RESOURCE_MANAGER.Textures.erase(Result->DepthPyramid->GetObjectID());

	Result->DepthPyramid->Bind();
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	const int MaxDimention = std::max(CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_R32F, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight));
	Result->DepthPyramid->Width = CameraComponent.RenderTargetWidth;
	Result->DepthPyramid->Height = CameraComponent.RenderTargetHeight;

	// ************************************ Bloom ************************************
	FEPostProcess* BloomEffect = ENGINE.CreatePostProcess("Bloom", static_cast<int>(CameraComponent.RenderTargetWidth / 4.0f), static_cast<int>(CameraComponent.RenderTargetHeight / 4.0f));
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
	FEPostProcess* GammaHDR = ENGINE.CreatePostProcess("GammaAndHDR", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	FEShader* GammaHDRShader = RESOURCE_MANAGER.GetShader("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/);
	GammaHDR->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, GammaHDRShader));
	RENDERER.AddPostProcess(Result, GammaHDR);
	// ************************************ Gamma & HDR END ************************************

	// ************************************ FXAA ************************************
	FEPostProcess* FEFXAAEffect = ENGINE.CreatePostProcess("FE_FXAA", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	FEShader* FEFXAAShader = RESOURCE_MANAGER.GetShader("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/);
	FEFXAAEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FEFXAAShader));
	RENDERER.AddPostProcess(Result, FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	Result->PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight));
	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FEPostProcess* DOFEffect = ENGINE.CreatePostProcess("DOF", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	FEShader* DOFShader = RESOURCE_MANAGER.GetShader("7800253C244442155D0F3C7B"/*"DOF"*/);
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, DOFShader));
	DOFEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));

	RENDERER.AddPostProcess(Result, DOFEffect);
	// ************************************ DOF END ************************************

	// ************************************ Chromatic Aberration ************************************
	FEPostProcess* ChromaticAberrationEffect = ENGINE.CreatePostProcess("chromaticAberration", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	FEShader* ChromaticAberrationShader = RESOURCE_MANAGER.GetShader("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/);
	ChromaticAberrationEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0 }, ChromaticAberrationShader));
	RENDERER.AddPostProcess(Result, ChromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	Result->PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight));
	// ************************************ Chromatic Aberration END ************************************

	return Result;
}

void FERenderer::OnResizeCameraRenderingDataUpdate(FEEntity* CameraEntity)
{
	if (CameraEntity == nullptr)
		return;

	FECameraRenderingData* CurrentData = nullptr;
	if (CameraRenderingDataMap.find(CameraEntity->GetObjectID()) != CameraRenderingDataMap.end())
	{
		CurrentData = GetCameraRenderingData(CameraEntity);
	}
	else // If it is fresh camera without rendering data.
	{
		CurrentData = GetCameraRenderingData(CameraEntity);
		return;
	}
		
	FECameraComponent& CameraComponent = CurrentData->CameraEntity->GetComponent<FECameraComponent>();
	delete CurrentData->SceneToTextureFB;
	CurrentData->SceneToTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);

	/*if (bVRActive)
	{
		if (VRScreenW != 0 && VRScreenH != 0)
		{
			delete SceneToVRTextureFB;
			SceneToVRTextureFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, VRScreenW, VRScreenH);
		}
	}*/

	if (CameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified)
		return;

	delete CurrentData->DepthPyramid;
	CurrentData->DepthPyramid = RESOURCE_MANAGER.CreateTexture();
	RESOURCE_MANAGER.Textures.erase(CurrentData->DepthPyramid->GetObjectID());

	CurrentData->DepthPyramid->Bind();
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	const int MaxDimention = std::max(CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);
	FE_GL_ERROR(glTexStorage2D(GL_TEXTURE_2D, static_cast<int>(MipCount), GL_R32F, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight));
	CurrentData->DepthPyramid->Width = CameraComponent.RenderTargetWidth;
	CurrentData->DepthPyramid->Height = CameraComponent.RenderTargetHeight;

	CurrentData->GBuffer->RenderTargetResize(CurrentData->SceneToTextureFB);
	CurrentData->SSAO->RenderTargetResize(CurrentData->SceneToTextureFB);

	for (size_t i = 0; i < CurrentData->PostProcessEffects.size(); i++)
	{
		// We should delete only internally created frame buffers.
		// Otherwise user created postProcess could create UB.
		if (CurrentData->PostProcessEffects[i]->GetName() == "Bloom" ||
			CurrentData->PostProcessEffects[i]->GetName() == "GammaAndHDR" ||
			CurrentData->PostProcessEffects[i]->GetName() == "FE_FXAA" ||
			CurrentData->PostProcessEffects[i]->GetName() == "DOF" ||
			CurrentData->PostProcessEffects[i]->GetName() == "chromaticAberration")
			delete CurrentData->PostProcessEffects[i];
	}

	CurrentData->PostProcessEffects.clear();

	// ************************************ Bloom ************************************
	FEPostProcess* BloomEffect = ENGINE.CreatePostProcess("Bloom", static_cast<int>(CameraComponent.RenderTargetWidth / 4.0f), static_cast<int>(CameraComponent.RenderTargetHeight / 4.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_SCENE_HDR_COLOR, RESOURCE_MANAGER.GetShader("0C19574118676C2E5645200E"/*"FEBloomThreshold"*/)));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 5.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 5.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 1.0f));

	BloomEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));
	BloomEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("BloomSize", 1.0f));

	BloomEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_HDR_COLOR}, RESOURCE_MANAGER.GetShader("1833272551376C2E5645200E"/*"FEBloomComposition"*/)));

	RENDERER.AddPostProcess(CurrentData, BloomEffect);
	// ************************************ Bloom END ************************************

	// ************************************ Gamma & HDR ************************************
	FEPostProcess* GammaHDR = ENGINE.CreatePostProcess("GammaAndHDR", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	GammaHDR->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("3417497A5E0C0C2A07456E44"/*"FEGammaAndHDRShader"*/)));
	RENDERER.AddPostProcess(CurrentData, GammaHDR);
	// ************************************ Gamma & HDR END ************************************

	// ************************************ FXAA ************************************
	FEPostProcess* FEFXAAEffect = ENGINE.CreatePostProcess("FE_FXAA", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	FEFXAAEffect->AddStage(new FEPostProcessStage(FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, RESOURCE_MANAGER.GetShader("1E69744A10604C2A1221426B"/*"FEFXAAShader"*/)));
	RENDERER.AddPostProcess(CurrentData, FEFXAAEffect);

	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	CurrentData->PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight));
	// ************************************ FXAA END ************************************

	// ************************************ DOF ************************************
	FEPostProcess* DOFEffect = ENGINE.CreatePostProcess("DOF", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, RESOURCE_MANAGER.GetShader("7800253C244442155D0F3C7B"/*"DOF"*/)));
	DOFEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(0.0f, 1.0f)));
	DOFEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0, FE_POST_PROCESS_SCENE_DEPTH}, RESOURCE_MANAGER.GetShader("7800253C244442155D0F3C7B"/*"DOF"*/)));
	DOFEffect->Stages.back()->StageSpecificUniformValues.push_back(FEShaderUniformValue("FEBlurDirection", glm::vec2(1.0f, 0.0f)));
	RENDERER.AddPostProcess(CurrentData, DOFEffect);
	// ************************************ DOF END ************************************

	// ************************************ chromaticAberrationEffect ************************************
	FEPostProcess* ChromaticAberrationEffect = ENGINE.CreatePostProcess("chromaticAberration", CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight);
	ChromaticAberrationEffect->AddStage(new FEPostProcessStage(std::vector<int> { FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0 }, RESOURCE_MANAGER.GetShader("9A41665B5E2B05321A332D09"/*"chromaticAberrationShader"*/)));
	RENDERER.AddPostProcess(CurrentData, ChromaticAberrationEffect);
	//#fix for now after gamma correction I assume that texture output should be GL_RGB but in future it should be changeable.
	CurrentData->PostProcessEffects.back()->ReplaceOutTexture(0, RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, CameraComponent.RenderTargetWidth, CameraComponent.RenderTargetHeight));
	// ************************************ chromaticAberrationEffect END ************************************
}

FECameraRenderingData* FERenderer::GetCameraRenderingData(FEEntity* CameraEntity)
{
	if (CameraEntity == nullptr)
		return nullptr;

	if (CameraRenderingDataMap.find(CameraEntity->GetObjectID()) != CameraRenderingDataMap.end())
	{
		return CameraRenderingDataMap[CameraEntity->GetObjectID()];
	}
	else
	{
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

	CreateCameraRenderingData(CameraEntity);
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

void FERenderer::Render(FEScene* CurrentScene)
{
	if (CurrentScene == nullptr)
		return;

	//  FIX ME!
	FEEntity* MainCameraEntity = CAMERA_SYSTEM.GetMainCamera(CurrentScene);
	if (MainCameraEntity == nullptr)
		return;

	CurrentCameraRenderingData = GetCameraRenderingData(MainCameraEntity);
	if (CurrentCameraRenderingData == nullptr)
		return;

	UpdateShadersForCamera(CurrentCameraRenderingData);
	LastRenderedSceneID = CurrentScene->GetObjectID();
	LastRenderedCameraID = MainCameraEntity->GetObjectID();

	FECameraComponent& CurrentCameraComponent = MainCameraEntity->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = MainCameraEntity->GetComponent<FETransformComponent>();

	FEViewport* CurrentViewport = CAMERA_SYSTEM.GetMainCameraViewport(CurrentScene);
	if (CurrentViewport != nullptr)
	{
		glm::ivec2 ViewportPosition = glm::ivec2(CurrentViewport->GetX(), CurrentViewport->GetY());
		glm::ivec2 ViewportSize = glm::ivec2(CurrentViewport->GetWidth(), CurrentViewport->GetHeight());

		MouseRay = GEOMETRY.CreateMouseRayToWorld(INPUT.GetMouseX(), INPUT.GetMouseY(),
												  CurrentCameraComponent.GetViewMatrix(), CurrentCameraComponent.GetProjectionMatrix(),
												  ViewportPosition, ViewportSize);
	}

	if (bVRActive)
		return;

	if (CurrentCameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified)
	{
		SimplifiedRender(CurrentScene);
		return;
	}

	CurrentCameraComponent.UpdateFrustumPlanes();

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
	entt::basic_view PrefabInstancedView = CurrentScene->Registry.view<FEInstancedComponent, FEPrefabInstanceComponent>();
	entt::basic_view TerrainView = CurrentScene->Registry.view<FETerrainComponent, FETransformComponent>();
	entt::basic_view VirtualUIView = CurrentScene->Registry.view<FEVirtualUIComponent, FETransformComponent>();

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

				for (entt::entity EnTTEntity : GameModelGroup)
				{
					auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

					if (!GameModelComponent.IsCastShadows() || !GameModelComponent.IsVisible())
						continue;

					FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
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

					if (!Entity->HasComponent<FEInstancedComponent>())
					{
						RenderGameModelComponent(Entity, nullptr, false);
					}
					else if (Entity->HasComponent<FEInstancedComponent>())
					{
						RenderGameModelComponentWithInstanced(Entity, nullptr, true, false);
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
						
						RenderGameModelComponentWithInstanced(Entity, nullptr, true, false, i);
						
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

	bUseOcclusionCulling = PreviousState;
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

	const unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, attachments);

	if (CurrentCameraComponent.IsClearColorEnabled())
	{
		glm::vec4 ClearColor = CurrentCameraComponent.GetClearColor();
		glClearColor(ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w);
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	UpdateGPUCullingFrustum();

	for (entt::entity EnTTEntity : GameModelGroup)
	{
		auto& [GameModelComponent, TransformComponent] = GameModelGroup.get<FEGameModelComponent, FETransformComponent>(EnTTEntity);

		if (!GameModelComponent.IsVisible() || !GameModelComponent.IsPostprocessApplied())
			continue;

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			ForceShader(RESOURCE_MANAGER.GetShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
			RenderGameModelComponent(Entity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{

			ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
			RenderGameModelComponentWithInstanced(Entity);
		}
	}

	for (auto [EnTTEntity, InstancedComponent, PrefabInstanceComponent] : PrefabInstancedView.each())
	{
		ForceShader(RESOURCE_MANAGER.GetShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);

		for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
		{
			RenderGameModelComponentWithInstanced(Entity, nullptr, false, false, i);
		}
	}

	for (auto [EnTTEntity, VirtualUIComponent, TransformComponent] : VirtualUIView.each())
	{
		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (Entity == nullptr)
			continue;

		//if (!VirtualUIComponent.IsVisible())
		//	continue;

		VIRTUAL_UI_SYSTEM.RenderVirtualUIComponent(Entity);
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

	CurrentCameraRenderingData->GBuffer->GFrameBuffer->UnBind();
	ForceShader(nullptr);

	CurrentCameraRenderingData->GBuffer->Albedo->Bind(0);
	CurrentCameraRenderingData->GBuffer->Normals->Bind(1);
	CurrentCameraRenderingData->GBuffer->MaterialProperties->Bind(2);
	CurrentCameraRenderingData->GBuffer->Positions->Bind(3);
	CurrentCameraRenderingData->GBuffer->ShaderProperties->Bind(4);
	
	// ************************************ SSAO ************************************
	UpdateSSAO();
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
	CurrentCameraRenderingData->SceneToTextureFB->Bind();

	const unsigned int attachments_[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments_);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* FinalSceneShader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	FinalSceneShader->Start();
	FinalSceneShader->UpdateUniformData("SSAOActive", CurrentCameraComponent.IsSSAOEnabled() ? 1.0f : 0.0f);
	LoadStandardUniforms(FinalSceneShader, true);
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
	InstancedLineShader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	InstancedLineShader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	InstancedLineShader->UpdateUniformData("resolution", glm::vec2(CurrentCameraRenderingData->SceneToTextureFB->GetWidth(), CurrentCameraRenderingData->SceneToTextureFB->GetHeight()));
	InstancedLineShader->LoadUniformsDataToGPU();

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
	CurrentCameraRenderingData->SceneToTextureFB->UnBind();

	//Generate the mipmaps of colorAttachment
	CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment()->Bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// ********* POST_PROCESS EFFECTS *********
	// Because we render post process effects with screen quad
	// we will turn off write to depth buffer in order to get clear DB to be able to render additional objects
	glDepthMask(GL_FALSE);
	CurrentCameraRenderingData->FinalScene = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();
	FETexture* PrevStageTex = CurrentCameraRenderingData->SceneToTextureFB->GetColorAttachment();

	for (size_t i = 0; i < CurrentCameraRenderingData->PostProcessEffects.size(); i++)
	{
		FEPostProcess& Effect = *CurrentCameraRenderingData->PostProcessEffects[i];
		for (size_t j = 0; j < Effect.Stages.size(); j++)
		{
			Effect.Stages[j]->Shader->Start();
			LoadStandardUniforms(Effect.Stages[j]->Shader, nullptr, nullptr);
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
					Effect.Stages[j]->InTexture[k] = PrevStageTex;
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

			PrevStageTex = Effect.Stages[j]->OutTexture;
		}
	}

	// Rendering the last effect to the default screen buffer.
	for (int i = static_cast<int>(CurrentCameraRenderingData->PostProcessEffects.size() - 1); i >= 0; i--)
	{
		FEPostProcess& Effect = *CurrentCameraRenderingData->PostProcessEffects[i];
		
		if (Effect.bActive)
		{
			if (CurrentCameraRenderingData->CameraEntity->GetComponent<FECameraComponent>().Viewport == ENGINE.GetDefaultViewport())
				Effect.RenderResult();
			// TO_DO: With introduction of multiple scenes\cameras, this code probably should be changed.
			// Temporary solution, now Engine will not render to default screen buffer, user would be responsible for it.
			// Maybe use ENGINE.GetRenderTargetMode() to determine if it should render to default screen buffer.
			//Effect.RenderResult();
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

		if (!GameModelComponent.IsVisible() || GameModelComponent.IsPostprocessApplied())
			continue;

		FEEntity* Entity = CurrentScene->GetEntityByEnTT(EnTTEntity);
		if (!Entity->HasComponent<FEInstancedComponent>())
		{
			RenderGameModelComponent(Entity);
		}
		else if (Entity->HasComponent<FEInstancedComponent>())
		{
			RenderGameModelComponentWithInstanced(Entity);
		}
	}
	
	CurrentCameraRenderingData->SceneToTextureFB->UnBind();
	CurrentCameraRenderingData->SceneToTextureFB->SetColorAttachment(OriginalColorAttachment);
	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI END *********

	// **************************** TERRAIN EDITOR TOOLS ****************************
	TERRAIN_SYSTEM.UpdateBrush(CurrentCameraTransformComponent.GetPosition(FE_WORLD_SPACE), MouseRay);
	// **************************** TERRAIN EDITOR TOOLS END ****************************

	LineCounter = 0;

	// **************************** DEPTH PYRAMID ****************************
#ifdef USE_OCCLUSION_CULLING

	ComputeTextureCopy->Start();
	ComputeTextureCopy->UpdateUniformData("textureSize", glm::vec2(CurrentCameraRenderingData->DepthPyramid->GetWidth(), CurrentCameraRenderingData->DepthPyramid->GetHeight()));
	ComputeTextureCopy->LoadUniformsDataToGPU();

	CurrentCameraRenderingData->SceneToTextureFB->GetDepthAttachment()->Bind(0);
	glBindImageTexture(1, CurrentCameraRenderingData->DepthPyramid->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

	ComputeTextureCopy->Dispatch(static_cast<unsigned>(ceil(float(CurrentCameraRenderingData->DepthPyramid->GetWidth()) / 32.0f)), static_cast<unsigned>(ceil(float(CurrentCameraRenderingData->DepthPyramid->GetHeight()) / 32.0f)), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const size_t MipCount = static_cast<size_t>(floor(log2(std::max(CurrentCameraRenderingData->DepthPyramid->GetWidth(), CurrentCameraRenderingData->DepthPyramid->GetHeight()))) + 1);
	for (size_t i = 0; i < MipCount; i++)
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

	CurrentCameraRenderingData = nullptr;
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

	FECameraRenderingData* CameraRenderingData = GetCameraRenderingData(MainCameraEntity);
	if (CameraRenderingData == nullptr)
	{
		LOG.Add("Camera rendering data is nullptr in FERenderer::SaveScreenshot", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	FETexture* CameraResult = RENDERER.GetCameraResult(MainCameraEntity);

	unsigned char* Pixels = new unsigned char[4 * CameraResult->GetWidth() * CameraResult->GetHeight()];
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, CameraResult->GetTextureID()));
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels));

	FETexture* TempTexture = RESOURCE_MANAGER.RawDataToFETexture(Pixels, CameraResult->GetWidth(), CameraResult->GetHeight());
	RESOURCE_MANAGER.SaveFETexture(TempTexture, FileName.c_str());
	RESOURCE_MANAGER.DeleteFETexture(TempTexture);

	delete[] Pixels;
}

void FERenderer::RenderGameModelComponent(FEEntity* Entity, FEEntity* ForceCamera, bool bReloadUniformBlocks)
{
	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();

	RenderGameModelComponent(GameModelComponent, TransformComponent, Entity->ParentScene, ForceCamera, bReloadUniformBlocks);
}

void FERenderer::RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FETransformComponent& TransformComponent, FEScene* ParentScene, FEEntity* ForceCamera, bool bReloadUniformBlocks)
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
	LoadStandardUniforms(GameModel->Material->Shader, GameModel->Material, &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	GameModel->Material->Shader->LoadUniformsDataToGPU();

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

// FIX ME! Should I have it as a separate function or it should be done only if camera is in forward rendering mode?
// That function is used only in VR rendering and Simplified !!
void FERenderer::RenderGameModelComponentForward(FEEntity* Entity, FEEntity* ForceCamera, bool bReloadUniformBlocks)
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

	//  FIX ME!
	FEEntity* CameraEntityToUse = ForceCamera;
	if (ForceCamera == nullptr)
	{
		CameraEntityToUse = TryToGetLastUsedCameraEntity();
		if (CameraEntityToUse == nullptr)
			return;
	}
	FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();

	FEShader* OriginalShader = nullptr;
	if (!(CurrentCameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified) || RENDERER.bVRActive)
	{
		OriginalShader = GameModel->Material->Shader;
		// FIX ME! It should not depend on RENDERER.bVRActive !
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
	LoadStandardUniforms(GameModel->Material->Shader, GameModel->Material, &TransformComponent, ForceCamera, GameModelComponent.IsReceivingShadows(), GameModelComponent.IsUniformLighting());
	GameModel->Material->Shader->LoadUniformsDataToGPU();

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

	if (!(CurrentCameraComponent.GetRenderingPipeline() == FERenderingPipeline::Forward_Simplified) || RENDERER.bVRActive)
		GameModel->Material->Shader = OriginalShader;

	if (GameModelComponent.IsWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	LoadStandardUniforms(TerrainComponent.Shader, nullptr, &TransformComponent, ForceCamera, TerrainComponent.IsReceivingShadows());
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
	if (TerrainComponent.Shader->GetUniform("usedLayersCount") != nullptr)
		TerrainComponent.Shader->UpdateUniformData("usedLayersCount", static_cast<float>(LayersUsed));
	// ************ Load materials data for all Terrain layers END ************

	TerrainComponent.Shader->UpdateUniformData("hightScale", TerrainComponent.HightScale);
	TerrainComponent.Shader->UpdateUniformData("scaleFactor", TerrainComponent.ScaleFactor);
	if (TerrainComponent.Shader->GetName() != "FESMTerrainShader")
		TerrainComponent.Shader->UpdateUniformData("tileMult", TerrainComponent.TileMult);
	TerrainComponent.Shader->UpdateUniformData("LODlevel", TerrainComponent.LODLevel);
	TerrainComponent.Shader->UpdateUniformData("hightMapShift", TerrainComponent.HightMapShift);

	glm::vec3 PivotPosition = TransformComponent.GetPosition();
	TerrainComponent.ScaleFactor = 1.0f * TerrainComponent.ChunkPerSide;

	static int PVMHash = static_cast<int>(std::hash<std::string>{}("FEPVMMatrix"));
	static int WorldMatrixHash = static_cast<int>(std::hash<std::string>{}("FEWorldMatrix"));
	static int HightMapShiftHash = static_cast<int>(std::hash<std::string>{}("hightMapShift"));

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


			TerrainComponent.Shader->UpdateUniformData("FEPVMMatrix", CurrentCameraComponent.GetProjectionMatrix() * CurrentCameraComponent.GetViewMatrix() * TransformComponent.GetWorldMatrix());
			if (TerrainComponent.Shader->GetUniform("FEWorldMatrix") != nullptr)
				TerrainComponent.Shader->UpdateUniformData("FEWorldMatrix", TransformComponent.GetWorldMatrix());
			TerrainComponent.Shader->UpdateUniformData("hightMapShift", glm::vec2(i * -1.0f, j * -1.0f));

			TerrainComponent.Shader->LoadUniformDataToGPU("FEPVMMatrix");

			if (TerrainComponent.Shader->GetUniform("FEWorldMatrix") != nullptr)
				TerrainComponent.Shader->LoadUniformDataToGPU("FEWorldMatrix");

			TerrainComponent.Shader->LoadUniformDataToGPU("hightMapShift");

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

void FERenderer::UpdateShadersForCamera(FECameraRenderingData* CameraData)
{
	if (CameraData == nullptr)
		return;

	FECameraComponent& CameraComponent = CameraData->CameraEntity->GetComponent<FECameraComponent>();
	if (CameraComponent.GetRenderingPipeline() != FERenderingPipeline::Deferred)
		return;
	
	// **************************** Bloom ********************************
	CameraData->PostProcessEffects[0]->Stages[0]->Shader->UpdateUniformData("thresholdBrightness", CameraComponent.GetBloomThreshold());

	CameraData->PostProcessEffects[0]->Stages[1]->StageSpecificUniformValues[1].SetValue(CameraComponent.GetBloomSize());
	CameraData->PostProcessEffects[0]->Stages[2]->StageSpecificUniformValues[1].SetValue(CameraComponent.GetBloomSize());
	// **************************** Bloom END ****************************

	// **************************** FXAA ********************************
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAASpanMax", CameraComponent.GetFXAASpanMax());
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAAReduceMin", CameraComponent.GetFXAAReduceMin());
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAAReduceMul", CameraComponent.GetFXAAReduceMul());
	CameraData->PostProcessEffects[2]->Stages[0]->Shader->UpdateUniformData("FXAATextureSize", glm::vec2(1.0f / CameraComponent.RenderTargetWidth, 1.0f / CameraComponent.RenderTargetHeight));
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

void FERenderer::GPUCulling(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, size_t PrefabIndex)
{
	if (bFreezeCulling)
		return;

	if (CurrentCameraRenderingData == nullptr)
	{
		FEEntity* CameraEntityToUse = TryToGetLastUsedCameraEntity();
		if (CameraEntityToUse == nullptr)
			return;

		CurrentCameraRenderingData = GetCameraRenderingData(CameraEntityToUse);
	}

	INSTANCED_RENDERING_SYSTEM.CheckDirtyFlag(EntityWithInstancedComponent);
	GPUCullingIndividual(EntityWithInstancedComponent, GameModelComponent, PrefabIndex);
}

void FERenderer::GPUCullingIndividual(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();

	FrustumCullingShader->Start();

	//  FIX ME!
	FEEntity* CameraEntityToUse = TryToGetLastUsedCameraEntity();
	if (CameraEntityToUse == nullptr)
		return;
	FECameraComponent& CurrentCameraComponent = CameraEntityToUse->GetComponent<FECameraComponent>();
	FETransformComponent& CurrentCameraTransformComponent = CameraEntityToUse->GetComponent<FETransformComponent>();

#ifdef USE_OCCLUSION_CULLING
	FrustumCullingShader->UpdateUniformData("FEProjectionMatrix", CurrentCameraComponent.GetProjectionMatrix());
	FrustumCullingShader->UpdateUniformData("FEViewMatrix", CurrentCameraComponent.GetViewMatrix());
	FrustumCullingShader->UpdateUniformData("useOcclusionCulling", bUseOcclusionCulling);
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

void FERenderer::UpdateSSAO()
{
	if (CurrentCameraRenderingData == nullptr)
		return;

	FECameraComponent& CameraComponent = CurrentCameraRenderingData->CameraEntity->GetComponent<FECameraComponent>();
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
	LoadStandardUniforms(CurrentCameraRenderingData->SSAO->Shader, true, nullptr);
	CurrentCameraRenderingData->SSAO->Shader->LoadUniformsDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/)->GetVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	CurrentCameraRenderingData->SSAO->Shader->Stop();

	if (CameraComponent.IsSSAOResultBlured())
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

void FERenderer::RenderVR(FEScene* CurrentScene)
{
	if (CurrentScene == nullptr || SceneToVRTextureFB == nullptr)
		return;

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

void FERenderer::AddAfterRenderCallback(std::function<void()> Callback)
{
	if (Callback == nullptr)
	{
		LOG.Add("Attempted to call FERenderer::AddAfterRenderCallback with Callback set to nullptr.", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	AfterRenderCallbacks.push_back(Callback);
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