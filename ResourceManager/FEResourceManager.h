#pragma once

#include "../Renderer/FEPostProcess.h"
#include "../ThirdParty/lodepng/lodepng.h"
#include "../ThirdParty/stb_image/stb_image.h"
#include "FEGLTFLoader.h"
#include "../FileSystem/FEAssetPackage.h"

#include "../SubSystems/Scene/FEPrefab.h"
#include "../SubSystems/Scene/Components/NativeScriptSystem/FENativeScriptModule.h"
#include "Config.h"

#define ENGINE_RESOURCE_TAG "ENGINE_PRIVATE_RESOURCE"

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEResourceManager
	{
		friend class FEngine;
		friend class FEScene;
		friend class FERenderer;
		friend class FETerrainSystem;
		friend class FESkyDomeSystem;
		friend class FEVirtualUIContext;
		friend class FEVirtualUISystem;
		friend class FENativeScriptSystem;
	public:
		SINGLETON_PUBLIC_PART(FEResourceManager)

		FEShader* CreateShader(std::string ShaderName, const char* VertexText, const char* FragmentText,
							   const char* TessControlText = nullptr, const char* TessEvalText = nullptr,
							   const char* GeometryText = nullptr, const char* ComputeText = nullptr, std::string ForceObjectID = "");

		FEShader* GetShader(std::string ShaderID);
		std::vector<FEShader*> GetShaderByName(std::string Name);
		std::vector<std::string> GetShaderIDList();
		std::vector<std::string> GetEnginePrivateShaderIDList();
		void DeleteShader(const FEShader* Shader);
		bool ReplaceShader(std::string OldShaderID, FEShader* NewShader);
		std::string LoadGLSL(const char* FileName);

		FETexture* LoadPNGTexture(const char* FileName, std::string Name = "");
		FETexture* LoadJPGTexture(const char* FileName, std::string Name = "");
		FETexture* LoadBMPTexture(const char* FileName, std::string Name = "");
		FETexture* LoadFETexture(const char* FileName, std::string Name = "", FETexture* ExistingTexture = nullptr);
		FETexture* LoadFETextureUnmanaged(const char* FileName, std::string Name = "");
		FETexture* LoadFETexture(char* FileData, std::string Name = "", FETexture* ExistingTexture = nullptr);
		FETexture* LoadFETextureAsync(const char* FileName, std::string Name = "", FETexture* ExistingTexture = nullptr, std::string ForceObjectID = "");
		FETexture* RawDataToFETexture(unsigned char* TextureData, int Width, int Height, GLint Internalformat = -1, GLenum Format = GL_RGBA, GLenum Type = GL_UNSIGNED_BYTE);
		std::vector<FETexture*> ChannelsToFETextures(FETexture* SourceTexture);
		unsigned char* ResizeTextureRawData(FETexture* SourceTexture, size_t TargetWidth, size_t TargetHeight, int FiltrationLevel = 0);
		unsigned char* ResizeTextureRawData(const unsigned char* TextureData, size_t Width, size_t Height, size_t TargetWidth, size_t TargetHeight, GLint InternalFormat, int FiltrationLevel = 0);
		void ResizeTexture(FETexture* SourceTexture, int TargetWidth, int TargetHeight, int FiltrationLevel = 0);
		FETexture* CreateTextureWithTransparency(FETexture* OriginalTexture, FETexture* MaskTexture);

		void SaveFETexture(FETexture* Texture, const char* FileName);
		bool ExportFETextureToPNG(FETexture* TextureToExport, const char* FileName);
		bool ExportRawDataToPNG(const char* FileName, const unsigned char* TextureData, int Width, int Height, GLint Internalformat);
		void DeleteFETexture(const FETexture* Texture);
		std::vector<std::string> GetTextureIDList();
		FETexture* GetTexture(std::string ID);
		std::vector<FETexture*> GetTextureByName(std::string Name);
		FETexture* NoTexture;
		FETexture* CreateTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateSameFormatTexture(FETexture* ReferenceTexture, int DifferentW = 0, int DifferentH = 0, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateCopyOfTexture(FETexture* ReferenceTexture, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateBlankHightMapTexture(int Width, int Height, std::string Name = "");
		void AddTextureToManaged(FETexture* Texture);

		FETexture* ImportTexture(const char* FileName);

		FEMesh* RawDataToMesh(std::vector<float>& Positions, std::vector<float>& Normals, std::vector<float>& Tangents, std::vector<float>& UV, std::vector<int>& Index, std::string Name = "");
		FEMesh* RawDataToMesh(float* Positions, int PosSize,
							  float* UV, int UVSize,
							  float* Normals, int NormSize,
							  float* Tangents, int TanSize,
							  int* Indices, int IndexSize,
							  float* Colors = nullptr, int ColorSize = 0,
							  float* MatIndexs = nullptr, int MatIndexsSize = 0, int MatCount = 0,
							  std::string Name = "");

		void DeleteFEMesh(const FEMesh* Mesh);

		std::vector<std::string> GetMeshIDList();
		std::vector<std::string> GetEnginePrivateMeshIDList();
		FEMesh* GetMesh(std::string ID);
		std::vector<FEMesh*> GetMeshByName(std::string Name);
		std::vector<FEObject*> ImportOBJ(const char* FileName, bool bForceOneMesh = false);
		FEMesh* LoadFEMesh(const char* FileName, std::string Name = "");
		void SaveFEMesh(FEMesh* Mesh, const char* FileName);
		void AddColorToFEMeshVertices(FEMesh* Mesh, float* Colors, int ColorSize);

		FEFramebuffer* CreateFramebuffer(int Attachments, int Width, int Height, bool bHDR = true);

		std::vector<std::string> GetMaterialIDList();
		std::vector<std::string> GetEnginePrivateMaterialIDList();
		FEMaterial* GetMaterial(std::string ID);
		std::vector<FEMaterial*> GetMaterialByName(std::string Name);
		FEMaterial* CreateMaterial(std::string Name = "", std::string ForceObjectID = "");
		Json::Value SaveMaterialToJSON(FEMaterial* Material);
		FEMaterial* LoadMaterialFromJSON(Json::Value& Root);
		void DeleteMaterial(const FEMaterial* Material);

		std::vector<std::string> GetGameModelIDList();
		std::vector<std::string> GetEnginePrivateGameModelIDList();
		FEGameModel* GetGameModel(std::string ID);
		std::vector<FEGameModel*> GetGameModelByName(std::string Name);
		FEGameModel* CreateGameModel(FEMesh* Mesh = nullptr, FEMaterial* Material = nullptr, std::string Name = "", std::string ForceObjectID = "");
		Json::Value SaveGameModelToJSON(FEGameModel* GameModel);
		FEGameModel* LoadGameModelFromJSON(Json::Value& Root);
		void DeleteGameModel(const FEGameModel* GameModel);

		std::vector<std::string> GetPrefabIDList();
		std::vector<std::string> GetEnginePrivatePrefabIDList();
		FEPrefab* GetPrefab(std::string ID);
		std::vector<FEPrefab*> GetPrefabByName(std::string Name);
		FEPrefab* CreatePrefab(std::string Name = "", std::string ForceObjectID = "", FEScene* SceneDescription = nullptr);
		Json::Value SavePrefabToJSON(FEPrefab* Prefab);
		FEPrefab* LoadPrefabFromJSON(Json::Value& Root);
		void DeletePrefab(const FEPrefab* Prefab);

		std::vector<std::string> GetNativeScriptModuleIDList();
		std::vector<std::string> GetEnginePrivateNativeScriptModuleIDList();
		FENativeScriptModule* GetNativeScriptModule(std::string ID);
		std::string ReadDLLModuleID(std::string DLLFilePath);
		std::vector<FENativeScriptModule*> GetNativeScriptModuleByName(std::string Name);
		FENativeScriptModule* CreateNativeScriptModule(std::string Name = "", std::string ForceObjectID = "");
		FENativeScriptModule* CreateNativeScriptModule(std::string DebugDLLFilePath, std::string DebugPDBFilePath, std::string ReleaseDLLFilePath, std::vector<std::string> ScriptFiles = {}, std::string Name = "", std::string ForceObjectID = "");
		FENativeScriptModule* LoadFENativeScriptModule(std::string FileName);
		void SaveFENativeScriptModule(FENativeScriptModule* NativeScriptModule, std::string FileName);

		FEAssetPackage* CreateEngineHeadersAssetPackage();
		bool UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, std::string Path);
		FEAssetPackage* CreateEngineSourceFilesAssetPackage();
		bool UnPackEngineSourceFilesAssetPackage(FEAssetPackage* AssetPackage, std::string Path);
		FEAssetPackage* CreateEngineLIBAssetPackage();
		bool UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, std::string Path);
		bool CopyEngineFiles(bool bCopyEngineHeaders, bool bCopyEngineSourceFiles, bool bCopyEngineLIBs, std::string DestinationDirectory);

		FEAssetPackage* CreatePrivateEngineAssetPackage();
		bool UnPackPrivateEngineAssetPackage(FEAssetPackage* AssetPackage, std::string Path);

		void Clear();
		void LoadStandardMeshes();
		void LoadStandardMaterial();
		void LoadStandardGameModels();

		void ReSaveEnginePrivateTextures();
		void ReSaveStandardMeshes();

		std::string GetDefaultResourcesFolder();

		// Returns true if the tag was set, false if the tag was not set.
		bool SetTag(FEObject* Object, std::string NewTag);

		Json::Value SaveFEObjectPart(FEObject* Object);
		FEObjectLoadedData LoadFEObjectPart(Json::Value Root);

		std::vector<std::string> GetTagsThatWillPreventDeletion();
		void AddTagThatWillPreventDeletion(std::string Tag);
		void RemoveTagThatWillPreventDeletion(std::string Tag);
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		std::unordered_map<std::string, FEShader*> Shaders;
		std::unordered_map<std::string, FETexture*> Textures;
		std::unordered_map<std::string, FEMaterial*> Materials;
		std::unordered_map<std::string, FEMesh*> Meshes;
		std::unordered_map<std::string, FEGameModel*> GameModels;
		std::unordered_map<std::string, FEPrefab*> Prefabs;
		std::unordered_map<std::string, FENativeScriptModule*> NativeScriptModules;

		FETexture* CreateTexture(std::string Name = "", std::string ForceObjectID = "");
		FEMesh* CreateMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name = "");

		FEPostProcess* CreatePostProcess(int ScreenWidth, int ScreenHeight, std::string Name);

		std::string GetFileNameFromFilePath(std::string FilePath);

		std::string FreeObjectName(FE_OBJECT_TYPE ObjectType);

		GLint MaxColorAttachments = 1;

		void CreateMaterialsFromOBJData(std::vector<FEObject*>& ResultArray);

		static void LoadTextureFileAsyncCallBack(void* OutputData);

		std::string EngineFolder = std::string(ENGINE_FOLDER) + "/";
		std::string ResourcesFolder = EngineFolder + "/Resources/";

		std::vector<std::string> TagsThatWillPreventDeletion = { ENGINE_RESOURCE_TAG };

		template<typename T>
		void ClearResource(std::unordered_map<std::string, T*>& ResourceMap);

		void SetTagIternal(FEObject* Object, std::string NewTag);

		template<typename T>
		std::vector<std::string> GetResourceIDListByTag(const std::unordered_map<std::string, T*>& Resources, const std::string& Tag);

		bool DeleteNativeScriptModuleInternal(FENativeScriptModule* Module);

		// FIX ME! It is temporary solution.
		bool bUsePackageForPrivateResources = false;
		FEAssetPackage* PrivateEngineAssetPackage = nullptr;
	};
#include "FEResourceManager.inl"

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetResourceManager();
	#define RESOURCE_MANAGER (*static_cast<FEResourceManager*>(GetResourceManager()))
#else
	#define RESOURCE_MANAGER FEResourceManager::GetInstance()
#endif
}