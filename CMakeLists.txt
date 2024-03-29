cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

# set the project name
project(FocalEngine LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "/MP /ZI /W3 /Od /MDd /JMC /sdl /FC /D IMGUI_IMPL_OPENGL_LOADER_GLEW /D WIN32 /D _WINDOWS /D _DEBUG /D FE_DEBUG_ENABLED")
set(CMAKE_CXX_FLAGS_RELEASE "/MP /W3 /O2 /Oi /Gy /sdl /FC /GL /OPT:REF /OPT:ICF /D IMGUI_IMPL_OPENGL_LOADER_GLEW /D NDEBUG /D WIN32 /D _WINDOWS")

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set default resource folder location
set(ENGINE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(${ENGINE_FOLDER}/ResourceManager/Config.h.in ${ENGINE_FOLDER}/ResourceManager/Config.h @ONLY)

if(NOT TARGET FEBasicApplication)
  add_subdirectory(SubSystems/FEBasicApplication)
endif()

file(GLOB InstancedLineMaterial_SRC
	"CoreExtensions/StandardMaterial/InstancedLineMaterial/FE_InstancedLine_FS.glsl"
	"CoreExtensions/StandardMaterial/InstancedLineMaterial/FE_InstancedLine_VS.glsl"
)

file(GLOB PBRMaterial_SRC
	"CoreExtensions/StandardMaterial/PBRMaterial/FE_PBR_FS.glsl"
	"CoreExtensions/StandardMaterial/PBRMaterial/FE_PBR_FS_DEFERRED.glsl"
	"CoreExtensions/StandardMaterial/PBRMaterial/FE_PBR_FS_GBUFFER.glsl"
	"CoreExtensions/StandardMaterial/PBRMaterial/FE_PBR_INSTANCED_VS.glsl"
	"CoreExtensions/StandardMaterial/PBRMaterial/FE_PBR_VS.glsl"
	"CoreExtensions/StandardMaterial/PBRMaterial/FE_PBR_VS_GBUFFER.glsl"
)

file(GLOB PhongMaterial_SRC
	"CoreExtensions/StandardMaterial/PhongMaterial/FE_Phong_FS.glsl"
	"CoreExtensions/StandardMaterial/PhongMaterial/FE_Phong_VS.glsl"
)

file(GLOB ShadowMapMaterial_SRC
	"CoreExtensions/StandardMaterial/ShadowMapMaterial/FE_ShadowMap_FS.glsl"
	"CoreExtensions/StandardMaterial/ShadowMapMaterial/FE_ShadowMap_INSTANCED_VS.glsl"
	"CoreExtensions/StandardMaterial/ShadowMapMaterial/FE_ShadowMap_VS.glsl"
)

file(GLOB SkyDomeMaterial_SRC
	"CoreExtensions/StandardMaterial/SkyDome/FE_SkyDome_FS.glsl"
	"CoreExtensions/StandardMaterial/SkyDome/FE_SkyDome_VS.glsl"
)

file(GLOB SolidColorMaterial_SRC
	"CoreExtensions/StandardMaterial/SolidColorMaterial/FE_SolidColor_FS.glsl"
	"CoreExtensions/StandardMaterial/SolidColorMaterial/FE_SolidColor_VS.glsl"
)

file(GLOB TerrainMaterial_SRC
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_Terrain_FS_DEFERRED.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_Terrain_FS_GBUFFER.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_Terrain_GS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_Terrain_TCS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_Terrain_TES.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_LayersNormalize_FS.glsl"
)

file(GLOB TerrainEditorTools_SRC
	"CoreExtensions/StandardMaterial/TerrainMaterial/EditTools/FE_BrushOutput_FS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/EditTools/FE_BrushOutput_VS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/EditTools/FE_BrushVisual_FS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/EditTools/FE_BrushVisual_VS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/EditTools/FE_LayersNormalize_FS.glsl"
)

file(GLOB TerrainShadowMapShader_SRC
	"CoreExtensions/StandardMaterial/TerrainMaterial/ShadowMapShader/FE_SMTerrain_FS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/ShadowMapShader/FE_SMTerrain_TCS.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/ShadowMapShader/FE_SMTerrain_TES.glsl"
	"CoreExtensions/StandardMaterial/TerrainMaterial/ShadowMapShader/FE_SMTerrain_VS.glsl"
)

file(GLOB PostProcessEffects_SRC
	"CoreExtensions/PostProcessEffects/FE_ScreenQuad_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_ScreenQuad_VS.glsl"
	"CoreExtensions/PostProcessEffects/FE_CombineFrameBuffers_FS.glsl"
)

file(GLOB FE_Bloom_SRC
	"CoreExtensions/PostProcessEffects/FE_Bloom/FE_Bloom_VS.glsl"
	"CoreExtensions/PostProcessEffects/FE_Bloom/FE_BloomBlur_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_Bloom/FE_BloomComposition_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_Bloom/FE_BloomThreshold_FS.glsl"
)

file(GLOB FE_ChromaticAberration_SRC
	"CoreExtensions/PostProcessEffects/FE_ChromaticAberration/FE_ChromaticAberration_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_ChromaticAberration/FE_ChromaticAberration_VS.glsl"
)

file(GLOB FE_DOF_SRC
	"CoreExtensions/PostProcessEffects/FE_DOF/FE_DOF_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_DOF/FE_DOF_VS.glsl"
)

file(GLOB FE_FXAA_SRC
	"CoreExtensions/PostProcessEffects/FE_FXAA/FE_FXAA_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_FXAA/FE_FXAA_VS.glsl"
)

file(GLOB FE_GammaAndHDRCorrection_SRC
	"CoreExtensions/PostProcessEffects/FE_GammaAndHDRCorrection/FE_Gamma_and_HDR_Correction_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_GammaAndHDRCorrection/FE_Gamma_and_HDR_Correction_VS.glsl"
)

file(GLOB FE_SSAO_SRC
	"CoreExtensions/PostProcessEffects/FE_SSAO/FE_SSAO_FS.glsl"
	"CoreExtensions/PostProcessEffects/FE_SSAO/FE_SSAO_VS.glsl"
	"CoreExtensions/PostProcessEffects/FE_SSAO/FE_SSAO_Blur_FS.glsl"
)

file(GLOB ComputeShaders_SRC
	"CoreExtensions/ComputeShaders/FE_FrustumCulling_CS.glsl"
	"CoreExtensions/ComputeShaders/FE_ComputeTextureCopy_CS.glsl"
	"CoreExtensions/ComputeShaders/FE_ComputeDepthPyramidDownSample_CS.glsl"
)

file(GLOB CoreExtensions_SRC
	"CoreExtensions/FEFreeCamera.cpp"
	"CoreExtensions/FEFreeCamera.h"
	"CoreExtensions/FEModelViewCamera.cpp"
	"CoreExtensions/FEModelViewCamera.h"
)

# *************** RENDERER ***************

file(GLOB Renderer_SRC
	"Renderer/FEEntity.cpp"
	"Renderer/FEEntity.h"
	"Renderer/FEEntityInstanced.cpp"
	"Renderer/FEEntityInstanced.h"
	"Renderer/FEPrefab.cpp"
	"Renderer/FEPrefab.h"
	"Renderer/FEFramebuffer.cpp"
	"Renderer/FEFramebuffer.h"
	"Renderer/FEGameModel.cpp"
	"Renderer/FEGameModel.h"
	"Renderer/FELight.cpp"
	"Renderer/FELight.h"
	"Renderer/FELine.cpp"
	"Renderer/FELine.h"
	"Renderer/FEMaterial.cpp"
	"Renderer/FEMaterial.h"
	"Renderer/FEMesh.cpp"
	"Renderer/FEMesh.h"
	"Renderer/FEPostProcess.cpp"
	"Renderer/FEPostProcess.h"
	"Renderer/FERenderer.cpp"
	"Renderer/FERenderer.h"
	"Renderer/FEShader.cpp"
	"Renderer/FEShader.h"
	"Renderer/FEShader.inl"
	"Renderer/FETexture.cpp"
	"Renderer/FETexture.h"
	"Renderer/FETerrain.cpp"
	"Renderer/FETerrain.h"
)

# *************** RESOURCE MANAGER ***************

file(GLOB ResourceManager_SRC
	"ResourceManager/FEObjLoader.cpp"
	"ResourceManager/FEObjLoader.h"
	"ResourceManager/FEglTFLoader.cpp"
	"ResourceManager/FEglTFLoader.h"
	"ResourceManager/FEResourceManager.cpp"
	"ResourceManager/FEResourceManager.h"
	"ResourceManager/Config.h"
)

# *************** SUB_SYSTEM ***************

file(GLOB SubSystems_SRC
	"SubSystems/FEBasicCamera.cpp"
	"SubSystems/FEBasicCamera.h"
	"SubSystems/FECoreIncludes.h"
	"SubSystems/FEFileSystem.cpp"
	"SubSystems/FEFileSystem.h"
	"SubSystems/FEGeometricTools.cpp"
	"SubSystems/FEGeometricTools.h"
	"SubSystems/FEObject.cpp"
	"SubSystems/FEObject.h"
	"SubSystems/FEScene.cpp"
	"SubSystems/FEScene.h"
	"SubSystems/FETransformComponent.cpp"
	"SubSystems/FETransformComponent.h"
)

file(GLOB OpenXR_SRC
	"SubSystems/FEOpenXR/FEOpenXRCore.h"
	"SubSystems/FEOpenXR/FEOpenXRCore.cpp"
	"SubSystems/FEOpenXR/FEOpenXRInput.h"
	"SubSystems/FEOpenXR/FEOpenXRInput.cpp"
	"SubSystems/FEOpenXR/FEOpenXRRendering.h"
	"SubSystems/FEOpenXR/FEOpenXRRendering.cpp"
	"SubSystems/FEOpenXR/FEOpenXR.h"
	"SubSystems/FEOpenXR/FEOpenXR.cpp"
)

# *************** THIRD_PARTY ***************

file(GLOB jsoncpp_SRC
	"ThirdParty/jsoncpp/json_reader.cpp"
	"ThirdParty/jsoncpp/json_tool.h"
	"ThirdParty/jsoncpp/json_value.cpp"
	"ThirdParty/jsoncpp/json_valueiterator.inl"
	"ThirdParty/jsoncpp/json_writer.cpp"
)

file(GLOB lodepng_SRC
	"ThirdParty/lodepng/lodepng.cpp"
	"ThirdParty/lodepng/lodepng.h"
)

file(GLOB stb_image_SRC
	"ThirdParty/stb_image/stb_image.h"
)

file(GLOB source_SRC
	"FEngine.cpp"
	"FEngine.h"
)

link_directories(${GLEW_LIB_DIR})
link_directories(${GLFW_LIB_DIR})

# add the executable
add_library(FocalEngine 
			${source_SRC}
			${InstancedLineMaterial_SRC}
			${PBRMaterial_SRC}
			${PhongMaterial_SRC}
			${ShadowMapMaterial_SRC}
			${SkyDomeMaterial_SRC}
			${SolidColorMaterial_SRC}
			${TerrainMaterial_SRC}
			${TerrainEditorTools_SRC}
			${TerrainShadowMapShader_SRC}
			${PostProcessEffects_SRC}
			${FE_Bloom_SRC}
			${FE_ChromaticAberration_SRC}
			${FE_DOF_SRC}
			${FE_FXAA_SRC}
			${FE_GammaAndHDRCorrection_SRC}
			${FE_SSAO_SRC}
			${ComputeShaders_SRC}
			${CoreExtensions_SRC}
			# *************** RENDERER ***************
			${Renderer_SRC}
			# *************** RESOURCE MANAGER ***************
			${ResourceManager_SRC}
			# *************** SUB_SYSTEM ***************
			${SubSystems_SRC}
			${OpenXR_SRC}
			# *************** THIRD_PARTY ***************
			${jsoncpp_SRC}
			${lodepng_SRC}
			${stb_image_SRC}
)

target_link_libraries(FocalEngine 
    PRIVATE 
        FEBasicApplication
        glew32s.lib
        glfw3.lib
        glfw3dll.lib
        opengl32.lib
    PUBLIC 
        debug	  ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/openxr_loaderd.lib
        optimized ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/openxr_loader.lib
)

source_group("Source Files" FILES ${source_SRC})
source_group("Source Files/CoreExtensions/Materials/InstancedLineMaterial" FILES ${InstancedLineMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/PBRMaterial" FILES ${PBRMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/PhongMaterial" FILES ${PhongMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/ShadowMapMaterial" FILES ${ShadowMapMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/SkyDomeMaterial" FILES ${SkyDomeMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/SolidColorMaterial" FILES ${SolidColorMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/TerrainMaterial" FILES ${TerrainMaterial_SRC})
source_group("Source Files/CoreExtensions/Materials/TerrainMaterial/EditTools" FILES ${TerrainEditorTools_SRC})
source_group("Source Files/CoreExtensions/Materials/TerrainMaterial/ShadowMapShader" FILES ${TerrainShadowMapShader_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects" FILES ${PostProcessEffects_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects/FE_Bloom" FILES ${FE_Bloom_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects/FE_ChromaticAberration" FILES ${FE_ChromaticAberration_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects/FE_DOF" FILES ${FE_DOF_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects/FE_FXAA" FILES ${FE_FXAA_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects/FE_GammaAndHDRCorrection" FILES ${FE_GammaAndHDRCorrection_SRC})
source_group("Source Files/CoreExtensions/PostProcessEffects/FE_SSAO" FILES ${FE_SSAO_SRC})
source_group("Source Files/CoreExtensions/ComputeShaders/" FILES ${ComputeShaders_SRC})
source_group("Source Files/CoreExtensions" FILES ${CoreExtensions_SRC})
# *************** RENDERER ***************
source_group("Source Files/Renderer" FILES ${Renderer_SRC})
# *************** RESOURCE MANAGER ***************
source_group("Source Files/ResourceManager" FILES ${ResourceManager_SRC})
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/FEOpenXR" FILES ${OpenXR_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/lodepng" FILES ${lodepng_SRC})
source_group("Source Files/ThirdParty/stb_image" FILES ${stb_image_SRC})
source_group("Source Files/ThirdParty/jsoncpp" FILES ${jsoncpp_SRC})

# set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FocalEngine)

set(FOCAL_ENGINE_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ PARENT_SCOPE)
set(FOCAL_ENGINE_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/)

include_directories(
	${GLEW_INCLUDE_DIR}
	${BASICAPP_THIRDPARTY_DIR}
	${BASICAPP_DIR}
	${FOCAL_ENGINE_THIRD_PARTY_DIR}
)

# Re-export of the variables from child projects
set(GLEW_LIB_DIR ${GLEW_LIB_DIR} PARENT_SCOPE)
set(GLFW_LIB_DIR ${GLFW_LIB_DIR} PARENT_SCOPE)

set(GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR} PARENT_SCOPE)
set(BASICAPP_THIRDPARTY_DIR ${BASICAPP_THIRDPARTY_DIR} PARENT_SCOPE)
set(BASICAPP_DIR ${BASICAPP_DIR} PARENT_SCOPE)
