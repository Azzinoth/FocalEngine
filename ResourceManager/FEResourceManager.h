#pragma once

#include "FEPLYManager.h"
#include "../Renderer/FEPostProcess.h"
#include "../ThirdParty/lodepng/lodepng.h"
#include "../ThirdParty/stb_image/stb_image.h"
#include "FEGLTFLoader.h"
#include "../FileSystem/FEAssetPackage.h"
#include "BaseResources/FELineCollection.h"
#include "BaseResources/FEPointCloud.h"
#include "../SubSystems/Scene/FEPrefab.h"
#include "../SubSystems/Scene/Components/NativeScriptSystem/FENativeScriptModule.h"

#include "../ThirdParty/laszip/laszip_api.h"
#include "Config.h"

#define ENGINE_RESOURCE_TAG "ENGINE_PRIVATE_RESOURCE"

namespace FocalEngine
{
	enum FE_DEPTH_EXPORT_MODE
	{
		FE_DEPTH_EXPORT_NONE = -1,
		FE_DEPTH_EXPORT_GRAYSCALE_PNG = 0,  // Normalized to [0,1], 8-bit grayscale PNG
		FE_DEPTH_EXPORT_16BIT_PNG = 1,      // Normalized to [0,1], 16-bit grayscale PNG  
		FE_DEPTH_EXPORT_32BIT_TIFF = 2      // FE_TO_DO: Implement raw float values, 32-bit float TIFF
	};

	class FOCAL_ENGINE_API FEResourceManager
	{
		friend class FETexture;
		friend class FEngine;
		friend class FEScene;
		friend class FERenderer;
		friend class FETerrainSystem;
		friend class FESkyDomeSystem;
		friend class FEVirtualUIContext;
		friend class FEVirtualUISystem;
		friend class FENativeScriptSystem;
		friend class FEPointCloudSystem;
		friend class FELineSystem;
		friend class FEVolumeSystem;
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
		std::string LoadGLSL(const std::string& FilePath);

		FETexture* LoadPNGTexture(const std::string& FilePath, std::string Name = "");
		FETexture* LoadJPGTexture(const std::string& FilePath, std::string Name = "");
		FETexture* LoadBMPTexture(const std::string& FilePath, std::string Name = "");
		FETexture* LoadFETexture(const std::string& FilePath, std::string Name = "", FETexture* ExistingTexture = nullptr);
		FETexture* LoadFETextureUnmanaged(const std::string& FilePath, std::string Name = "");
		FETexture* LoadFETexture(char* FileData, std::string Name = "", FETexture* ExistingTexture = nullptr);
		FETexture* LoadFETextureAsync(const std::string& FilePath, std::string Name = "", FETexture* ExistingTexture = nullptr, std::string ForceObjectID = "");
		FETexture* RawDataToFETexture(unsigned char* TextureData, int Width, int Height, GLint Internalformat = -1, GLenum Format = GL_RGBA, GLenum Type = GL_UNSIGNED_BYTE);
		FETexture* RawDataTo3DFETexture(unsigned char* TextureData, int Width, int Height, int Depth, GLint Internalformat = -1, GLenum Format = GL_RGBA, GLenum Type = GL_UNSIGNED_BYTE);
		std::vector<FETexture*> ChannelsToFETextures(FETexture* SourceTexture);
		FETexture* Convert3DTextureToFlipbook2D(FETexture* Source3DTexture, int& ColumnsOut, int& RowsOut, std::string Name = "");
		FETexture* ConvertFlipbook2DTo3DTexture(FETexture* Source2DTexture, int Columns, int Rows, std::string Name = "");
		unsigned char* ResizeTextureRawData(FETexture* SourceTexture, size_t TargetWidth, size_t TargetHeight, int FiltrationLevel = 0);
		unsigned char* ResizeTextureRawData(const unsigned char* TextureData, size_t Width, size_t Height, size_t TargetWidth, size_t TargetHeight, GLint InternalFormat, int FiltrationLevel = 0);
		void ResizeTexture(FETexture* SourceTexture, int TargetWidth, int TargetHeight, int FiltrationLevel = 0);
		FETexture* CreateTextureWithTransparency(FETexture* OriginalTexture, FETexture* MaskTexture);

		void SaveFETexture(FETexture* Texture, const std::string& FilePath);
		bool ExportFETextureToPNG(FETexture* TextureToExport, const std::string& FilePath, FE_DEPTH_EXPORT_MODE DepthExportMode = FE_DEPTH_EXPORT_GRAYSCALE_PNG);
		bool ExportRawDataToPNG(const std::string& FilePath, const unsigned char* TextureData, int Width, int Height, GLint Internalformat);
		void DeleteFETexture(const FETexture* Texture);
		std::vector<std::string> GetTextureIDList();
		FETexture* GetTexture(std::string ID);
		std::vector<FETexture*> GetTextureByName(std::string Name);
		FETexture* NoTexture;
		FETexture* CreateTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateSameFormatTexture(FETexture* ReferenceTexture, int DifferentW = 0, int DifferentH = 0, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateCopyOfTexture(FETexture* ReferenceTexture, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateBlankHeightMapTexture(int Width, int Height, std::string Name = "");
		void AddTextureToManaged(FETexture* Texture);
		FETexture* Create3DTexture(GLint InternalFormat, GLenum Format, int Width, int Height, int Depth, bool bUnManaged = true, std::string Name = "");

		FETexture* ImportTexture(const std::string& FilePath);

		FEMesh* RawDataToMesh(std::vector<float>& Positions, std::vector<float>& Normals, std::vector<float>& Tangents, std::vector<float>& UV, std::vector<int>& Index, std::string Name = "");
		FEMesh* RawDataToMesh(float* Positions, int PositionsCount,
							  float* UV, int UVCount,
							  float* Normals, int NormalsCount,
							  float* Tangents, int TangentsCount,
							  int* Indices, int IndicesCount,
							  float* Colors = nullptr, int ColorsCount = 0,
							  float* MaterialIndices = nullptr, int MaterialIndicesCount = 0, int MaterialCount = 0,
							  std::string Name = "");
		FEMesh* RawPLYDataToFEMesh(FERawPLYData* PLYData, std::string Name = "", std::string ForceObjectID = "");

		void DeleteFEMesh(const FEMesh* Mesh);
		bool ExportFEMeshToOBJ(FEMesh* MeshToExport, const std::string& FilePath);
		bool ExportFEMeshToPLY(FEMesh* MeshToExport, const std::string& FilePath);

		std::vector<std::string> GetMeshIDList();
		std::vector<std::string> GetEnginePrivateMeshIDList();
		FEMesh* GetMesh(std::string ID);
		std::vector<FEMesh*> GetMeshByName(std::string Name);
		std::vector<FEObject*> ImportOBJ(const std::string& FilePath, bool bForceOneMesh = false);
		
		FEMesh* LoadFEMesh(const std::string& FilePath, std::string Name = "");
		void SaveFEMesh(FEMesh* Mesh, const std::string& FilePath);
		void AddColorToFEMeshVertices(FEMesh* Mesh, float* Colors, int ColorSize);
		// FE_FIX_ME: Remove this function. And redo vertex attributes management.
		// Data of such user defined vertex attributes will not be stored along FEMesh.
		void SetUserDataVertexAttributeActive(FEMesh* Mesh);

		FELineCollection* RawDataToFELineCollection(std::vector<FELine> Lines, std::string Name = "");
		void DeleteFELineCollection(const FELineCollection* LineCollection);

		//bool ExportLineCollectionToShapeFile(FELineCollection* LineCollectionToExport, const std::string& FilePath);

		std::vector<std::string> GetFELineCollectionIDList();
		std::vector<std::string> GetEnginePrivateFELineCollectionIDList();
		FELineCollection* GetLineCollection(std::string ID);
		std::vector<FELineCollection*> GetLineCollectionByName(std::string Name);

		FELineCollection* LoadFELineCollection(const std::string& FilePath, std::string Name = "");
		void SaveFELineCollection(FELineCollection* LineCollection, const std::string& FilePath);

		std::vector<std::string> GetPointCloudIDList();
		std::vector<std::string> GetEnginePrivatePointCloudIDList();
		FEPointCloud* GetPointCloud(std::string ID);
		std::vector<FEPointCloud*> GetPointCloudByName(std::string Name);
		FEPointCloud* RawDataToFEPointCloud(std::vector<FEPointCloudVertexDouble>& RawPointCloudDataDouble, std::string Name = "", std::string ForceObjectID = "", bool bCenterPositions = true, bool bAdvancedRendering = false, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor = nullptr);
		FEPointCloud* RawDataToFEPointCloud(std::vector<FEPointCloudVertex>& RawPointCloudData, std::string Name = "", std::string ForceObjectID = "", bool bCenterPositions = true, bool bAdvancedRendering = false, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor = nullptr);
		FEPointCloud* RawPLYDataToFEPointCloud(FERawPLYData* PLYData, std::string Name = "", std::string ForceObjectID = "", bool bCenterPositions = true, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor = nullptr);
		bool ReadLasOrLaz(const std::string& FilePath, std::vector<FEPointCloudVertexDouble>& RawData, laszip_header* OutHeaderCopy = nullptr);
		FEPointCloud* LasOrLazToFEPointCloud(const std::string& FilePath, std::string Name = "", std::string ForceObjectID = "", bool bCenterPositions = true, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor = nullptr, laszip_header* OutHeaderCopy = nullptr);
		FEPointCloud* ImportPointCloud(const std::string& FilePath, std::function<void(std::vector<FEPointCloudVertex>& RawData)> UserDataProcessor = nullptr);
		void ImportLasOrLazPointCloudAsync(const std::string& FilePath, std::function<void(FEPointCloud*)> CallBack, bool bCenterPositions = true, std::function<void(std::vector<FEPointCloudVertexDouble>& RawData)> UserDataProcessor = nullptr, laszip_header* OutHeaderCopy = nullptr);
		FEPointCloud* LoadFEPointCloud(const std::string& FilePath, std::string Name = "");
		void SaveFEPointCloud(FEPointCloud* PointCloud, const std::string& FilePath);
		bool SaveRawDataToPLY(std::vector<FEPointCloudVertex>& RawData, const std::string& FilePath);
		bool ExportFEPointCloudToPLY(FEPointCloud* PointCloudToExport, const std::string& FilePath);
		bool SaveRawDataToLASOrLAZ(std::vector<FEPointCloudVertex>& RawData, const std::string& FilePath, bool bIsCompressed = true, double ScaleFactor = 0.001);
		bool ExportFEPointCloudToLAS(FEPointCloud* PointCloudToExport, const std::string& FilePath);
		bool ExportFEPointCloudToLAZ(FEPointCloud* PointCloudToExport, const std::string& FilePath);
		void DeleteFEPointCloud(FEPointCloud* PointCloud);

		FEFramebuffer* CreateFramebuffer();
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
		FENativeScriptModule* LoadFENativeScriptModule(const std::string& FilePath);
		void SaveFENativeScriptModule(FENativeScriptModule* NativeScriptModule, const std::string& FilePath);

		FEAssetPackage* CreateEngineHeadersAssetPackage();
		bool UnPackEngineHeadersAssetPackage(FEAssetPackage* AssetPackage, const std::string& DirectoryPath);
		FEAssetPackage* CreateEngineSourceFilesAssetPackage();
		bool UnPackEngineSourceFilesAssetPackage(FEAssetPackage* AssetPackage, const std::string& DirectoryPath);
		FEAssetPackage* CreateEngineLIBAssetPackage();
		bool UnPackEngineLIBAssetPackage(FEAssetPackage* AssetPackage, const std::string& DirectoryPath);
		bool CopyEngineFiles(bool bCopyEngineHeaders, bool bCopyEngineSourceFiles, bool bCopyEngineLIBs, const std::string& DestinationDirectoryPath);

		FEAssetPackage* CreatePrivateEngineAssetPackage();
		bool UnPackPrivateEngineAssetPackage(FEAssetPackage* AssetPackage, const std::string& DirectoryPath);

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
		FEObjectLoadedData LoadFEObjectPart(const Json::Value& Root);

		std::vector<std::string> GetTagsThatWillPreventDeletion();
		void AddTagThatWillPreventDeletion(std::string Tag);
		void RemoveTagThatWillPreventDeletion(std::string Tag);

		FEObject* ImportPLYFile(const std::string& FilePath);
		bool DoesPLYContainMesh(FERawPLYData* PLYData);
		bool DoesPLYContainPointCloud(FERawPLYData* PLYData);

		std::string GetEngineFolder();

		glm::dvec3 GetLastLoadedMeshAppliedShift();
		glm::dvec3 GetLastLoadedPointCloudAppliedShift();
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		std::unordered_map<std::string, FEShader*> Shaders;
		std::unordered_map<std::string, FETexture*> Textures;
		std::unordered_map<std::string, FEMaterial*> Materials;
		std::unordered_map<std::string, FEMesh*> Meshes;
		std::unordered_map<std::string, FELineCollection*> LineCollections;
		std::unordered_map<std::string, FEPointCloud*> PointClouds;
		std::unordered_map<std::string, FEGameModel*> GameModels;
		std::unordered_map<std::string, FEPrefab*> Prefabs;
		std::unordered_map<std::string, FENativeScriptModule*> NativeScriptModules;

		FETexture* CreateTexture(std::string Name = "", std::string ForceObjectID = "");
		void Upload2DTextureDataToGPU(FETexture* Texture, GLint Level, GLint Internalformat, GLsizei Width, GLsizei Height, GLenum Format, GLenum DataType, const void* Data);
		void Upload3DTextureDataToGPU(FETexture* Texture, GLint Level, GLint Internalformat, GLsizei Width, GLsizei Height, GLsizei Depth, GLenum Format, GLenum DataType, const void* Data);
		FEMesh* CreateMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name = "");

		FEPostProcess* CreatePostProcess(int ScreenWidth, int ScreenHeight, std::string Name);

		std::string FreeObjectName(FE_OBJECT_TYPE ObjectType);

		GLint MaxColorAttachments = 1;

		void CreateMaterialsFromOBJData(std::vector<FEObject*>& ResultArray);

		static void LoadTextureFileAsyncCallBack(void* OutputData);

		std::string EngineFolder = std::string(ENGINE_FOLDER) + "/";
		std::string ResourcesFolder = EngineFolder + "/Resources/";

		std::vector<std::string> TagsThatWillPreventDeletion = { ENGINE_RESOURCE_TAG };

		template<typename T>
		void ClearResource(std::unordered_map<std::string, T*>& ResourceMap);

		void SetTagInternal(FEObject* Object, std::string NewTag);

		template<typename T>
		std::vector<std::string> GetResourceIDListByTag(const std::unordered_map<std::string, T*>& Resources, const std::string& Tag);

		bool DeleteNativeScriptModuleInternal(FENativeScriptModule* Module);

		// FE_TO_DO: Find a better way to handle resource extraction.
		// These variables are used to extract engine resources after application build.
		bool bUsePackageForPrivateResources = false;
		FEAssetPackage* PrivateEngineAssetPackage = nullptr;

		std::variant<std::vector<glm::vec3>, std::vector<glm::dvec3>> ExtractPositionsFromPLYData(FERawPLYData* PLYData);
		std::vector<int> ExtractIndicesFromPLYData(FERawPLYData* PLYData);
		std::vector<std::vector<unsigned char>> ExtractColorsFromPLYData(FERawPLYData* PLYData);
		std::vector<glm::vec2> ExtractUVsFromPLYData(FERawPLYData* PLYData, bool& bTextureCoordinatesArePartOfVertex);
		std::vector<glm::vec3> ExtractNormalsFromPLYData(FERawPLYData* PLYData);

		bool bIsLasLazFilesEnabled = false;
		bool SetUpPointCloudGPUBuffers(FEPointCloud* PointCloud, std::vector<FEPointCloudVertex>& RawPointCloudData);

		static void LoadPointCloudFileAsyncCallBack(void* OutputData);
		glm::dvec3 LastPointCloudAppliedShift;
	};
#include "FEResourceManager.inl"

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetResourceManager();
	#define RESOURCE_MANAGER (*static_cast<FEResourceManager*>(GetResourceManager()))
#else
	#define RESOURCE_MANAGER FEResourceManager::GetInstance()
#endif
}