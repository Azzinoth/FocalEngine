#pragma once

#include "../Core/FECoreIncludes.h"

namespace FocalEngine
{
	namespace FEEngineResourceIDs
	{
		// Shaders.
		inline constexpr FEUUID PBRShader = FEUUID::from_string("d4c93483-387b-501e-9b45-fe2edfc6998b").value();
		inline constexpr FEUUID PBRShaderForward = FEUUID::from_string("06348184-767c-5583-af28-df8fd2fe3d52").value();
		inline constexpr FEUUID PBRGBufferShader = FEUUID::from_string("c6de6537-57bb-5805-93fb-58363cd37edd").value();
		inline constexpr FEUUID PBRInstancedShader = FEUUID::from_string("0dd399ea-f5d4-5f28-b8eb-4c006bbb736f").value();
		inline constexpr FEUUID PBRInstancedGBufferShader = FEUUID::from_string("f9a62f55-59df-5a9a-810d-7918033ba67b").value();
		inline constexpr FEUUID SolidColorShader = FEUUID::from_string("12e5bc5a-7d95-5f2b-9c37-40248fb4f06f").value();
		inline constexpr FEUUID ScreenQuadShader = FEUUID::from_string("6166c2eb-9d13-56f6-af5e-9c97b0485346").value();
		inline constexpr FEUUID CombineFrameBuffersShader = FEUUID::from_string("1489555e-9551-5164-a71d-047a893b5bc1").value();
		inline constexpr FEUUID BloomThresholdShader = FEUUID::from_string("a11eeea2-c471-5d89-aa9c-0ea484f72541").value();
		inline constexpr FEUUID BloomBlurShader = FEUUID::from_string("5aac5207-da7a-5cf9-80a7-84ddbd000d12").value();
		inline constexpr FEUUID BloomCompositionShader = FEUUID::from_string("7aeb2e0f-f1f8-5cf2-98dd-472a6c70490a").value();
		inline constexpr FEUUID GammaAndHDRShader = FEUUID::from_string("6494ce17-4897-54a9-979d-cc5e1d488165").value();
		inline constexpr FEUUID FXAAShader = FEUUID::from_string("0eb9880a-4b73-5365-8b10-c1c8f0c566b6").value();
		inline constexpr FEUUID DOFShader = FEUUID::from_string("dab10856-5b8e-5ea4-bcd3-3666ee6826db").value();
		inline constexpr FEUUID ChromaticAberrationShader = FEUUID::from_string("8c723491-5d6e-5d25-9c97-b0e3d02f1b75").value();
		inline constexpr FEUUID SSAOShader = FEUUID::from_string("08c8d0f2-a00c-517f-bae8-197e99f016cc").value();
		inline constexpr FEUUID SSAOBlurShader = FEUUID::from_string("1916e4fa-f063-5504-9174-c310fb3690c8").value();
		inline constexpr FEUUID TerrainShader = FEUUID::from_string("666f7c29-1404-56a3-89d0-e0278fdb475a").value();
		inline constexpr FEUUID SMTerrainShader = FEUUID::from_string("1b88ef5e-1a9c-5b7a-9028-21b88a03004e").value();
		inline constexpr FEUUID TerrainBrushOutputShader = FEUUID::from_string("f18a3b5e-26da-5675-aed8-7772620b7e0f").value();
		inline constexpr FEUUID TerrainBrushVisualShader = FEUUID::from_string("53ba209f-6dab-5360-8cea-8d9f9495bbc3").value();
		inline constexpr FEUUID TerrainLayersNormalizeShader = FEUUID::from_string("4e0c43d3-9073-539e-a315-53fa58bb608c").value();
		inline constexpr FEUUID PhongShader = FEUUID::from_string("1d971bac-13ea-510b-aba9-dcfe7956285f").value();
		inline constexpr FEUUID InstancedLineShader = FEUUID::from_string("1824a7e6-8f04-5648-8069-e24970caa7b7").value();
		inline constexpr FEUUID SkyDomeShader = FEUUID::from_string("3bc9cee1-3c7c-5ca1-b645-792d9c6498ea").value();
		inline constexpr FEUUID VirtualUICanvasShader = FEUUID::from_string("4fbcc234-75b8-562d-995d-5986074ca936").value();
		inline constexpr FEUUID VolumetricShaderBasic = FEUUID::from_string("5c214c2c-2b48-521b-b2ed-a35cd57a2ef5").value();
		inline constexpr FEUUID VolumetricShaderCloudLike = FEUUID::from_string("c70024ea-ebf6-53ca-accb-2c2f3fd15dc8").value();
		inline constexpr FEUUID VolumetricShaderBasicAnimated = FEUUID::from_string("1af6f9dc-ef9b-50f9-89c7-92d96a7b2c5f").value();

		// Meshes.
		inline constexpr FEUUID PlaneMesh = FEUUID::from_string("4c15443a-5824-595f-b5ed-25d2022c13f0").value();
		inline constexpr FEUUID SphereMesh = FEUUID::from_string("0419567a-0ebc-5153-a08e-65321bc57109").value();
		inline constexpr FEUUID CubeMesh = FEUUID::from_string("db609958-a353-5c6c-9529-03e842e68ad7").value();
		inline constexpr FEUUID GenericVRControllerMesh = FEUUID::from_string("a270a383-cd29-5852-ad65-d31f1291be12").value();

		// Materials.
		inline constexpr FEUUID SolidColorMaterial = FEUUID::from_string("58e37694-4f98-5db0-a5c3-6ab47d41cc10").value();
		inline constexpr FEUUID SkyDomeMaterial = FEUUID::from_string("11905b5d-0e60-5822-be0d-3cf7d4e34c93").value();
		inline constexpr FEUUID GenericVRControllerMaterial = FEUUID::from_string("ece66eb9-eb9d-5c56-b531-5d22de2131e7").value();
		inline constexpr FEUUID ShadowMapMaterial = FEUUID::from_string("5101b358-9090-58a1-9833-f5ec51662470").value();
		inline constexpr FEUUID ShadowMapMaterialInstanced = FEUUID::from_string("33e26164-8b75-58e7-ae52-255a42fa9ecf").value();
		inline constexpr FEUUID PBRBaseMaterial = FEUUID::from_string("2a4b37f7-1cb8-5b82-8b57-f3c515f5aee6").value();

		// Textures.
		inline constexpr FEUUID NoTexture = FEUUID::from_string("972234c3-34d5-59d2-a15e-856cf6c3fba9").value();

		// Game models.
		inline constexpr FEUUID StandardGameModel = FEUUID::from_string("a5e423df-d923-5365-a0d6-22a5f62bb78f").value();
		inline constexpr FEUUID GenericVRControllerGameModel = FEUUID::from_string("e993317b-8991-5b35-a564-2f1b7b775a57").value();
		inline constexpr FEUUID SkyDomeGameModel = FEUUID::from_string("53ba921b-f9f1-50cb-ba05-a068f89f2d6f").value();

		// Prefabs.
		inline constexpr FEUUID FreeCameraPrefab = FEUUID::from_string("ff018bb9-0478-5dbe-96be-2b000e4ffb36").value();
		inline constexpr FEUUID ModelViewCameraPrefab = FEUUID::from_string("a72e898d-7fb3-58a9-9506-add92d7492b7").value();

		// Native script modules.
		inline constexpr FEUUID CameraScriptsModule = FEUUID::from_string("e1ff1a0e-000b-5f87-9b08-dcec07635c17").value();
	}
}
