cmake_minimum_required(VERSION 3.1)

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
	"CoreExtensions/StandardMaterial/TerrainMaterial/FE_Terrain_FS.glsl"
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

# *************** EDITOR ***************
file(GLOB Editor_SRC
	"Editor/FEEditor.cpp"
	"Editor/FEEditor.h"
	"Editor/FEEditorContentBrowser.cpp"
	"Editor/FEEditorDragAndDropManager.cpp"
	"Editor/FEEditorDragAndDropManager.h"
	"Editor/FEEditorGizmoManager.cpp"
	"Editor/FEEditorGizmoManager.h"
	"Editor/FEEditorHaloSelectionEffect.cpp"
	"Editor/FEEditorHaloSelectionEffect.h"
	"Editor/FEEditorInternalResources.cpp"
	"Editor/FEEditorInternalResources.h"
	"Editor/FEEditorPreviewManager.cpp"
	"Editor/FEEditorPreviewManager.h"
	"Editor/FEEditorSelectedObject.cpp"
	"Editor/FEEditorSelectedObject.h"
	"Editor/FEEditorVirtualFileSystem.cpp"
	"Editor/FEEditorVirtualFileSystem.h"
	"Editor/FEProject.cpp"
	"Editor/FEProject.h"
)

file(GLOB FEDearImguiWrapper_SRC
	"Editor/FEDearImguiWrapper/FEDearImguiWrapper.cpp"
	"Editor/FEDearImguiWrapper/FEDearImguiWrapper.h"
)

file(GLOB FEEditorSubWindows_SRC
	"Editor/FEEditorSubWindows/deletePopups.cpp"
	"Editor/FEEditorSubWindows/deletePopups.h"
	"Editor/FEEditorSubWindows/editPopups.cpp"
	"Editor/FEEditorSubWindows/editPopups.h"
	"Editor/FEEditorSubWindows/gyzmosSettingsWindow.h"
	"Editor/FEEditorSubWindows/combineChannelsToTexturePopUp.cpp"
	"Editor/FEEditorSubWindows/combineChannelsToTexturePopUp.h"
	"Editor/FEEditorSubWindows/projectWasModifiedPopUp.cpp"
	"Editor/FEEditorSubWindows/projectWasModifiedPopUp.h"
	"Editor/FEEditorSubWindows/renamePopups.cpp"
	"Editor/FEEditorSubWindows/renamePopups.h"
	"Editor/FEEditorSubWindows/selectPopups.cpp"
	"Editor/FEEditorSubWindows/selectPopups.h"
	"Editor/FEEditorSubWindows/shaderEditorWindow.cpp"
	"Editor/FEEditorSubWindows/shaderEditorWindow.h"
	"Editor/FEEditorSubWindows/resizeTexturePopup.cpp"
	"Editor/FEEditorSubWindows/resizeTexturePopup.h"
	"Editor/FEEditorSubWindows/prefabEditorWindow.cpp"
	"Editor/FEEditorSubWindows/prefabEditorWindow.h"
	"Editor/FEEditorSubWindows/debugTextureViewWindow.cpp"
	"Editor/FEEditorSubWindows/debugTextureViewWindow.h"
)

file(GLOB Editor_Materials_SRC
	"Editor/Materials/FE_HaloDrawObject_FS.glsl"
	"Editor/Materials/FE_HaloDrawObject_INSTANCED_VS.glsl"
	"Editor/Materials/FE_HaloDrawObject_VS.glsl"
	"Editor/Materials/FE_HaloSelectionEffect_FS.glsl"
	"Editor/Materials/FE_MeshPreview_FS.glsl"
	"Editor/Materials/FE_MeshPreview_VS.glsl"
	"Editor/Materials/FE_PixelAccurateSelection_FS.glsl"
	"Editor/Materials/FE_PixelAccurateSelection_INSTANCED_VS.glsl"
	"Editor/Materials/FE_PixelAccurateSelection_VS.glsl"
)

file(GLOB FEVisualNodeSystem_SRC
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNodeSystem.cpp"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNodeSystem.h"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNodeArea.cpp"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNodeArea.h"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNode.cpp"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNode.h"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNodeSocket.cpp"
	"Editor/NodeSystem/FEVisualNodeSystem/FEVisualNodeSocket.h"
)

file(GLOB FEVisualNodeSystem_CustomNodes_SRC
	"Editor/NodeSystem/CustomNodes/FEEditorStandardNodes.h"
	"Editor/NodeSystem/CustomNodes/FEEditorMaterialNode.cpp"
	"Editor/NodeSystem/CustomNodes/FEEditorMaterialNode.h"
	"Editor/NodeSystem/CustomNodes/FEEditorFloatSourceNode.cpp"
	"Editor/NodeSystem/CustomNodes/FEEditorFloatSourceNode.h"
	"Editor/NodeSystem/CustomNodes/FEEditorTextureSourceNode.cpp"
	"Editor/NodeSystem/CustomNodes/FEEditorTextureSourceNode.h"
	"Editor/NodeSystem/CustomNodes/FEEditorTextureCreatingNode.cpp"
	"Editor/NodeSystem/CustomNodes/FEEditorTextureCreatingNode.h"
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

file(GLOB BasicApplicationSubSystem_SRC
	"SubSystems/FEBasicApplication/FEBasicApplication.cpp"
	"SubSystems/FEBasicApplication/FEBasicApplication.h"
	"SubSystems/FEBasicApplication/FEThreadPool.cpp"
	"SubSystems/FEBasicApplication/FEThreadPool.h"
	"SubSystems/FEBasicApplication/FELog.cpp"
	"SubSystems/FEBasicApplication/FELog.h"
	"SubSystems/FEBasicApplication/FETime.cpp"
	"SubSystems/FEBasicApplication/FETime.h"
)

# *************** THIRD_PARTY ***************

file(GLOB imgui_SRC
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_demo.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_draw.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_impl_glfw.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_impl_opengl3.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_tables.cpp"
	"SubSystems/FEBasicApplication/ThirdParty/imgui/imgui_widgets.cpp"
)

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

file(GLOB textEditor_SRC
	"ThirdParty/textEditor/TextEditor.cpp"
)

file(GLOB stb_image_SRC
	"ThirdParty/stb_image/stb_image.h"
)

file(GLOB source_SRC
	"FEngine.cpp"
	"FEngine.h"
	"main.cpp"
)

link_directories("SubSystems/FEBasicApplication/ThirdParty/glew2/lib/$(PlatformTarget)")
link_directories("SubSystems/FEBasicApplication/ThirdParty/GLFW/lib/$(PlatformTarget)")

# add the executable
add_executable(FocalEngineEditor WIN32
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
								 # *************** EDITOR ***************
								 ${Editor_SRC}
								 ${FEDearImguiWrapper_SRC}
								 ${FEEditorSubWindows_SRC}
								 ${Editor_Materials_SRC}
								 ${FEVisualNodeSystem_SRC}
								 ${FEVisualNodeSystem_CustomNodes_SRC}
								 # *************** RENDERER ***************
								 ${Renderer_SRC}
								 # *************** RESOURCE MANAGER ***************
								 ${ResourceManager_SRC}
								 # *************** SUB_SYSTEM ***************
								 ${SubSystems_SRC}
								 ${BasicApplicationSubSystem_SRC}
								 # *************** THIRD_PARTY ***************
								 ${imgui_SRC}
								 ${jsoncpp_SRC}
								 ${lodepng_SRC}
								 ${textEditor_SRC}
								 ${stb_image_SRC}
)

target_link_libraries(FocalEngineEditor glew32.lib)
target_link_libraries(FocalEngineEditor glfw3.lib)
target_link_libraries(FocalEngineEditor glfw3dll.lib)
target_link_libraries(FocalEngineEditor opengl32.lib)

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
# *************** EDITOR ***************
source_group("Source Files/Editor" FILES ${Editor_SRC})
source_group("Source Files/Editor/FEDearImguiWrapper" FILES ${FEDearImguiWrapper_SRC})
source_group("Source Files/Editor/FEEditorSubWindows" FILES ${FEEditorSubWindows_SRC})
source_group("Source Files/Editor/Materials" FILES ${Editor_Materials_SRC})
source_group("Source Files/Editor/NodeSystem/FEVisualNodeSystem" FILES ${FEVisualNodeSystem_SRC})
source_group("Source Files/Editor/NodeSystem/CustomNodes" FILES ${FEVisualNodeSystem_CustomNodes_SRC})
# *************** RENDERER ***************
source_group("Source Files/Renderer" FILES ${Renderer_SRC})
# *************** RESOURCE MANAGER ***************
source_group("Source Files/ResourceManager" FILES ${ResourceManager_SRC})
# *************** SUB_SYSTEM ***************
source_group("Source Files/SubSystems" FILES ${SubSystems_SRC})
source_group("Source Files/SubSystems/BasicApplication" FILES ${BasicApplicationSubSystem_SRC})
# *************** THIRD_PARTY ***************
source_group("Source Files/ThirdParty/imgui" FILES ${imgui_SRC})
source_group("Source Files/ThirdParty/jsoncpp" FILES ${jsoncpp_SRC})
source_group("Source Files/ThirdParty/lodepng" FILES ${lodepng_SRC})
source_group("Source Files/ThirdParty/textEditor" FILES ${textEditor_SRC})
source_group("Source Files/ThirdParty/stb_image" FILES ${stb_image_SRC})

# set FocalEngineEditor as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FocalEngineEditor)

include_directories(
	"SubSystems/FEBasicApplication/ThirdParty/glew2/include"
	"SubSystems/FEBasicApplication/ThirdParty/"
	"SubSystems/FEBasicApplication/"
	"ThirdParty/"
)