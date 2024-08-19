cmake_minimum_required(VERSION 3.10)

set(BUILD_TYPE "Debug and Release" CACHE STRING "Choose Build type")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

# set the project name
project(FocalEngine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  # Disable C4251 warning
  add_compile_options(/wd4251)

  # Disable LNK4197 warning
  add_link_options("/ignore:4197")
endif()

option(ENABLE_TESTS "Enable tests" OFF)

set(TEST_LIBS)
set(TEST_SOURCE_FILES)
set(TEST_INCLUDE_FILES)

if(ENABLE_TESTS)
    list(APPEND TEST_LIBS
        debug       Tests/ThirdParty/googletest/lib/Debug/gtest
        debug       Tests/ThirdParty/googletest/lib/Debug/gtest_main
        optimized   Tests/ThirdParty/googletest/lib/Release/gtest
        optimized   Tests/ThirdParty/googletest/lib/Release/gtest_main
    )

	list(APPEND TEST_SOURCE_FILES
        "Tests/RunAllTests.cpp"
        "Tests/RunAllTests.h"
		"Tests/SceneGraphTest.cpp"
        "Tests/SceneGraphTest.h"
    )

	list(APPEND TEST_INCLUDE_FILES
        "Tests/ThirdParty/googletest/include"
    )

	# Adding flags for multi-processor compilation
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
endif()

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Any project that will include FocalEngine will need to redefine this variable
# to make sure that the engine can find its dependencies
set(ENGINE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}")
set(ENGINE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
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

# *************** RENDERER ***************

file(GLOB Renderer_SRC
	"Renderer/FEPrefab.cpp"
	"Renderer/FEPrefab.h"
	"Renderer/FEFramebuffer.cpp"
	"Renderer/FEFramebuffer.h"
	"Renderer/FEGameModel.cpp"
	"Renderer/FEGameModel.h"
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
)

# *************** RESOURCE MANAGER ***************

file(GLOB ResourceManager_SRC
	"ResourceManager/FEObjLoader.cpp"
	"ResourceManager/FEObjLoader.h"
	"ResourceManager/FEglTFLoader.cpp"
	"ResourceManager/FEglTFLoader.h"
	"ResourceManager/FEResourceManager.cpp"
	"ResourceManager/FEResourceManager.inl"
	"ResourceManager/FEResourceManager.h"
	"ResourceManager/Config.h"
)

# *************** CORE ***************

file(GLOB Core_SRC
	"Core/FocalEngineAPI.h"
	"Core/FECoreIncludes.h"
	"Core/FEGeometricTools.cpp"
	"Core/FEGeometricTools.h"
	"Core/FEObject.cpp"
	"Core/FEObject.h"
)

# *************** COMPONENTS ***************

file(GLOB Scene_Components_SRC
	"SubSystems/Scene/Components/FEComponents.h"
	"SubSystems/Scene/Components/FEComponents.cpp"
	"SubSystems/Scene/Components/FETagComponent.cpp"
	"SubSystems/Scene/Components/FETagComponent.h"
	"SubSystems/Scene/Components/FETransformComponent.cpp"
	"SubSystems/Scene/Components/FETransformComponent.h"
	"SubSystems/Scene/Components/FECameraComponent.cpp"
	"SubSystems/Scene/Components/FECameraComponent.h"
	"SubSystems/Scene/Components/FELightComponent.cpp"
	"SubSystems/Scene/Components/FELightComponent.h"
	"SubSystems/Scene/Components/FEGameModelComponent.cpp"
	"SubSystems/Scene/Components/FEGameModelComponent.h"
	"SubSystems/Scene/Components/FEInstancedComponent.cpp"
	"SubSystems/Scene/Components/FEInstancedComponent.h"
	"SubSystems/Scene/Components/FETerrainComponent.cpp"
	"SubSystems/Scene/Components/FETerrainComponent.h"
	"SubSystems/Scene/Components/FESkyDomeComponent.cpp"
	"SubSystems/Scene/Components/FESkyDomeComponent.h"
	"SubSystems/Scene/Components/FEPrefabInstanceComponent.h"
	"SubSystems/Scene/Components/FEPrefabInstanceComponent.cpp"
	"SubSystems/Scene/Components/FEVirtualUIComponent.h"
	"SubSystems/Scene/Components/FEVirtualUIComponent.cpp"
)

file(GLOB Scene_Component_Systems_SRC
	"SubSystems/Scene/Components/Systems/FEComponentSystems.h"
	"SubSystems/Scene/Components/Systems/FETransformSystem.h"
	"SubSystems/Scene/Components/Systems/FETransformSystem.cpp"
	"SubSystems/Scene/Components/Systems/FELightSystem.h"
	"SubSystems/Scene/Components/Systems/FELightSystem.cpp"
	"SubSystems/Scene/Components/Systems/FECameraSystem.h"
	"SubSystems/Scene/Components/Systems/FECameraSystem.cpp"
	"SubSystems/Scene/Components/Systems/FEInstancedSystem.h"
	"SubSystems/Scene/Components/Systems/FEInstancedSystem.cpp"
	"SubSystems/Scene/Components/Systems/FETerrainSystem.h"
	"SubSystems/Scene/Components/Systems/FETerrainSystem.cpp"
	"SubSystems/Scene/Components/Systems/FESkyDomeSystem.h"
	"SubSystems/Scene/Components/Systems/FESkyDomeSystem.cpp"
	"SubSystems/Scene/Components/Systems/FEPrefabInstanceSystem.h"
	"SubSystems/Scene/Components/Systems/FEPrefabInstanceSystem.cpp"
	"SubSystems/Scene/Components/Systems/FEVirtualUISystem.h"
	"SubSystems/Scene/Components/Systems/FEVirtualUISystem.cpp"
)

# *************** SCENE ***************

file(GLOB Scene_SRC
	"SubSystems/Scene/FESceneManager.cpp"
	"SubSystems/Scene/FESceneManager.inl"
	"SubSystems/Scene/FESceneManager.h"
	"SubSystems/Scene/FEScene.cpp"
	"SubSystems/Scene/FEScene.inl"
	"SubSystems/Scene/FEScene.h"
	"SubSystems/Scene/FENaiveSceneGraph.cpp"
	"SubSystems/Scene/FENaiveSceneGraph.inl"
	"SubSystems/Scene/FENaiveSceneGraph.h"
	"SubSystems/Scene/FENaiveSceneGraphNode.cpp"
	"SubSystems/Scene/FENaiveSceneGraphNode.h"
	"SubSystems/Scene/FEEntity.cpp"
	"SubSystems/Scene/FEEntity.h"
)

# *************** SUB_SYSTEM ***************

file(GLOB SubSystems_SRC
	"SubSystems/FEFileSystem.cpp"
	"SubSystems/FEFileSystem.h"
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

set(ALL_SOURCE_FILES "")
list(APPEND ALL_SOURCE_FILES
			${TEST_SOURCE_FILES}
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
			${Core_SRC}
			# *************** RENDERER ***************
			${Renderer_SRC}
			# *************** RESOURCE MANAGER ***************
			${ResourceManager_SRC}
			# *************** SUB_SYSTEM ***************
			${SubSystems_SRC}
			${Scene_SRC}
			${Scene_Components_SRC}
			${Scene_Component_Systems_SRC}
			${OpenXR_SRC}
			# *************** THIRD_PARTY ***************
			${jsoncpp_SRC}
			${lodepng_SRC}
			${stb_image_SRC}
)

if(ENABLE_TESTS)
    add_executable(FocalEngine WIN32 ${ALL_SOURCE_FILES})
else()
    if(BUILD_SHARED_LIBS)
		add_library(FocalEngine SHARED ${ALL_SOURCE_FILES})
		target_compile_definitions(FocalEngine PRIVATE FOCALENGINE_EXPORTS)
		target_compile_definitions(FocalEngine PRIVATE FEBASICAPPLICATION_SHARED)
		target_compile_definitions(FocalEngine PUBLIC FOCALENGINE_SHARED)

		set_target_properties(FocalEngine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
	else()
		add_library(FocalEngine STATIC ${ALL_SOURCE_FILES})
	endif()
endif()

# Adjust OpenXRLoader linking based on BUILD_SHARED_LIBS and FEBASICAPPLICATION_STATIC_RUNTIME
if(BUILD_SHARED_LIBS)
    if(USE_STATIC_RUNTIME)
        message(WARNING "Building a shared library with static runtime is uncommon and may lead to issues.")
        set(OpenXRLoader_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/DLL/Static_runtime/openxr_loaderd.lib)
        set(OpenXRLoader_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/DLL/Static_runtime/openxr_loader.lib)

		set(OpenXRLoader_DLL_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/DLL/Static_runtime/openxr_loaderd.dll)
        set(OpenXRLoader_DLL_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/DLL/Static_runtime/openxr_loader.dll)
    else()
        set(OpenXRLoader_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/DLL/Dynamic_runtime/openxr_loaderd.lib)
        set(OpenXRLoader_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/DLL/Dynamic_runtime/openxr_loader.lib)

		set(OpenXRLoader_DLL_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/DLL/Dynamic_runtime/openxr_loaderd.dll)
        set(OpenXRLoader_DLL_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/DLL/Dynamic_runtime/openxr_loader.dll)
    endif()
else()
    if(USE_STATIC_RUNTIME)
        set(OpenXRLoader_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/Static_runtime/openxr_loaderd.lib)
        set(OpenXRLoader_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/Static_runtime/openxr_loader.lib)

		# No DLL for static library
        set(OpenXRLoader_DLL_Debug "")
        set(OpenXRLoader_DLL_Release "")
    else()
        set(OpenXRLoader_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/Dynamic_runtime/openxr_loaderd.lib)
        set(OpenXRLoader_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/Dynamic_runtime/openxr_loader.lib)

		set(OpenXRLoader_DLL_Debug   ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Debug/DLL/Dynamic_runtime/openxr_loaderd.dll)
        set(OpenXRLoader_DLL_Release ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/openxr/Lib/x64/Release/DLL/Dynamic_runtime/openxr_loader.dll)
    endif()
endif()

# Set OpenXRLoader_DLL based on the current configuration
set(OpenXRLoader_DLL $<$<CONFIG:Debug>:${OpenXRLoader_DLL_Debug}>$<$<CONFIG:Release>:${OpenXRLoader_DLL_Release}>)

# Define GLEW_STATIC for static builds
if(USE_STATIC_RUNTIME)
    add_definitions(-DGLEW_STATIC)
endif()

target_link_libraries(FocalEngine 
    PRIVATE 
        FEBasicApplication
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        opengl32.lib
    PUBLIC 
        debug	  ${OpenXRLoader_Debug}
        optimized ${OpenXRLoader_Release}
		${TEST_LIBS}
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(FocalEngine PRIVATE FEBASICAPPLICATION_SHARED)
endif()

if(ENABLE_TESTS)
	source_group("Test Source Files" FILES ${TEST_SOURCE_FILES})
endif()

source_group("Source Files" FILES ${source_SRC})
source_group("Source Files/Core" FILES ${Core_SRC})
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
# *************** RENDERER ***************
source_group("Source Files/Renderer" FILES ${Renderer_SRC})
# *************** RESOURCE MANAGER ***************
source_group("Source Files/ResourceManager" FILES ${ResourceManager_SRC})
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/Scene" FILES ${Scene_SRC})
source_group("Source Files/SubSystems/Scene/Components" FILES ${Scene_Components_SRC})
source_group("Source Files/SubSystems/Scene/Components/Systems" FILES ${Scene_Component_Systems_SRC})
source_group("Source Files/SubSystems/FEOpenXR" FILES ${OpenXR_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/lodepng" FILES ${lodepng_SRC})
source_group("Source Files/ThirdParty/stb_image" FILES ${stb_image_SRC})
source_group("Source Files/ThirdParty/jsoncpp" FILES ${jsoncpp_SRC})

# set the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FocalEngine)

set(FOCAL_ENGINE_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ PARENT_SCOPE)
set(FOCAL_ENGINE_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/)

set(FOCAL_ENGINE_ENTT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/entt PARENT_SCOPE)
set(FOCAL_ENGINE_ENTT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/entt)

set(FOCAL_ENGINE_INCLUDES "")
list(APPEND FOCAL_ENGINE_INCLUDES
    ${FOCAL_ENGINE_THIRD_PARTY_DIR}
    ${FOCAL_ENGINE_ENTT_DIR}
)
set(FOCAL_ENGINE_INCLUDES ${FOCAL_ENGINE_INCLUDES} PARENT_SCOPE)

include_directories(
	${GLEW_INCLUDE_DIR}
	${BASICAPP_THIRDPARTY_DIR}
	${BASICAPP_DIR}
	${FOCAL_ENGINE_INCLUDES}
if(ENABLE_TESTS)
	${TEST_INCLUDE_FILES}
endif()
)

# Re-export of the variables from child projects
set(GLEW_LIB_DIR ${GLEW_LIB_DIR} PARENT_SCOPE)
set(GLFW_LIB_DIR ${GLFW_LIB_DIR} PARENT_SCOPE)

set(GLEW_INCLUDE_DIR ${GLEW_INCLUDE_DIR} PARENT_SCOPE)
set(BASICAPP_THIRDPARTY_DIR ${BASICAPP_THIRDPARTY_DIR} PARENT_SCOPE)
set(BASICAPP_DIR ${BASICAPP_DIR} PARENT_SCOPE)

# Export the BUILD_SHARED_LIBS and USE_STATIC_RUNTIME variables
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} PARENT_SCOPE)
set(USE_STATIC_RUNTIME ${USE_STATIC_RUNTIME} PARENT_SCOPE)

# Export the GLEW_LIBRARY and GLFW_LIBRARY variables
set(GLEW_LIBRARY ${GLEW_LIBRARY} PARENT_SCOPE)
set(GLFW_LIBRARY ${GLFW_LIBRARY} PARENT_SCOPE)

# Export the OpenXRLoader variable
set(OpenXRLoader_DLL ${OpenXRLoader_DLL} PARENT_SCOPE)

# If building as DLL, copy necessary DLLs to output directory
if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET FocalEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLFW_LIB_DIR}/glfw3.dll"
            "$<TARGET_FILE_DIR:FocalEngine>"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:FEBasicApplication>/FEBasicApplication.dll"
            "$<TARGET_FILE_DIR:FocalEngine>"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${OpenXRLoader_DLL}
			"$<TARGET_FILE_DIR:FocalEngine>"
    )
endif()

# If using dynamic runtime, copy necessary DLLs to output directory
if(NOT USE_STATIC_RUNTIME)
    add_custom_command(TARGET FocalEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLEW_LIB_DIR}/glew32.dll"
            "$<TARGET_FILE_DIR:FocalEngine>"
    )
endif()