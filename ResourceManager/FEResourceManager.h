#pragma once

#include "../Renderer/FEPostProcess.h"
#include "../Renderer/FETerrain.h"
#include "../ThirdParty/lodepng/lodepng.h"
#include "../ThirdParty/stb_image/stb_image.h"
#include "FEGLTFLoader.h"
#include "Config.h"

namespace FocalEngine
{
	class FEngine;
	class FEScene;
	class FERenderer;

	class FEResourceManager
	{
		friend FEngine;
		friend FEScene;
		friend FERenderer;
	public:
		SINGLETON_PUBLIC_PART(FEResourceManager)

		FEShader* CreateShader(std::string ShaderName, const char* VertexText, const char* FragmentText,
							   const char* TessControlText = nullptr, const char* TessEvalText = nullptr,
							   const char* GeometryText = nullptr, const char* ComputeText = nullptr, std::string ForceObjectID = "");

		bool MakeShaderStandard(FEShader* Shader);
		FEShader* GetShader(std::string ShaderID);
		std::vector<FEShader*> GetShaderByName(std::string Name);
		std::vector<std::string> GetShadersList();
		std::vector<std::string> GetStandardShadersList();
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
		FETexture* LoadPNGHeightmap(const char* FileName, FETerrain* Terrain, std::string Name = "");
		FETexture* LoadFEHeightmap(const char* FileName, FETerrain* Terrain, std::string Name = "");
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
		std::vector<std::string> GetTextureList();
		FETexture* GetTexture(std::string ID);
		std::vector<FETexture*> GetTextureByName(std::string Name);
		bool MakeTextureStandard(FETexture* Texture);
		FETexture* NoTexture;
		FETexture* CreateTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool bUnManaged = true, std::string Name = "");
		FETexture* CreateSameFormatTexture(FETexture* ExampleTexture, int DifferentW = 0, int DifferentH = 0, bool bUnManaged = true, std::string Name = "");
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
		bool MakeMeshStandard(FEMesh* Mesh);

		std::vector<std::string> GetMeshList();
		std::vector<std::string> GetStandardMeshList();
		FEMesh* GetMesh(std::string ID);
		std::vector<FEMesh*> GetMeshByName(std::string Name);
		std::vector<FEObject*> ImportOBJ(const char* FileName, bool bForceOneMesh = false);
		FEMesh* LoadFEMesh(const char* FileName, std::string Name = "");
		void SaveFEMesh(FEMesh* Mesh, const char* FileName);
		void AddColorToFEMeshVertices(FEMesh* Mesh, float* Colors, int ColorSize);

		FEFramebuffer* CreateFramebuffer(int Attachments, int Width, int Height, bool bHDR = true);

		std::vector<std::string> GetMaterialList();
		std::vector<std::string> GetStandardMaterialList();
		FEMaterial* GetMaterial(std::string ID);
		std::vector<FEMaterial*> GetMaterialByName(std::string Name);
		FEMaterial* CreateMaterial(std::string Name = "", std::string ForceObjectID = "");
		bool MakeMaterialStandard(FEMaterial* Material);
		void DeleteMaterial(const FEMaterial* Material);

		std::vector<std::string> GetGameModelList();
		std::vector<std::string> GetStandardGameModelList();
		FEGameModel* GetGameModel(std::string ID);
		std::vector<FEGameModel*> GetGameModelByName(std::string Name);
		FEGameModel* CreateGameModel(FEMesh* Mesh = nullptr, FEMaterial* Material = nullptr, std::string Name = "", std::string ForceObjectID = "");
		bool MakeGameModelStandard(FEGameModel* GameModel);
		void DeleteGameModel(const FEGameModel* GameModel);

		std::vector<std::string> GetPrefabList();
		std::vector<std::string> GetStandardPrefabList();
		FEPrefab* GetPrefab(std::string ID);
		std::vector<FEPrefab*> GetPrefabByName(std::string Name);
		FEPrefab* CreatePrefab(FEGameModel* GameModel = nullptr, std::string Name = "", std::string ForceObjectID = "");
		bool MakePrefabStandard(FEPrefab* Prefab);
		void DeletePrefab(const FEPrefab* Prefab);

		FETerrain* CreateTerrain(bool bCreateHeightMap = true, std::string Name = "", std::string ForceObjectID = "");
		void ActivateTerrainVacantLayerSlot(FETerrain* Terrain, FEMaterial* Material);
		void LoadTerrainLayerMask(const char* FileName, FETerrain* Terrain, size_t LayerIndex);
		void SaveTerrainLayerMask(const char* FileName, const FETerrain* Terrain, size_t LayerIndex);
		void FillTerrainLayerMask(const FETerrain* Terrain, size_t LayerIndex);
		void ClearTerrainLayerMask(const FETerrain* Terrain, size_t LayerIndex);
		void DeleteTerrainLayerMask(FETerrain* Terrain, size_t LayerIndex);

		void Clear();
		void LoadStandardMeshes();
		void LoadStandardMaterial();
		void LoadStandardGameModels();
		void LoadStandardPrefabs();

		void ReSaveStandardTextures();
		void ReSaveStandardMeshes();

		std::string GetDefaultResourcesFolder();
		//std::vector<FEObject*> LoadGLTF(const char* FileName);

		
		//std::vector<FEObject*> ImportAsset(const char* FileName);
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		std::unordered_map<std::string, FEShader*> Shaders;
		std::unordered_map<std::string, FEShader*> StandardShaders;

		FETexture* CreateTexture(std::string Name = "", std::string ForceObjectID = "");
		FEMesh* CreateMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name = "");

		FEPostProcess* CreatePostProcess(int ScreenWidth, int ScreenHeight, std::string Name);
		std::unordered_map<std::string, FETexture*> Textures;
		std::unordered_map<std::string, FETexture*> StandardTextures;

		std::unordered_map<std::string, FEMaterial*> Materials;
		std::unordered_map<std::string, FEMaterial*> StandardMaterials;

		std::unordered_map<std::string, FEMesh*> Meshes;
		std::unordered_map<std::string, FEMesh*> StandardMeshes;

		std::unordered_map<std::string, FEGameModel*> GameModels;
		std::unordered_map<std::string, FEGameModel*> StandardGameModels;

		std::unordered_map<std::string, FEPrefab*> Prefabs;
		std::unordered_map<std::string, FEPrefab*> StandardPrefabs;

		std::string GetFileNameFromFilePath(std::string FilePath);
		FEEntity* CreateEntity(FEGameModel* GameModel, std::string Name, std::string ForceObjectID = "");
		FEEntity* CreateEntity(FEPrefab* Prefab, std::string Name, std::string ForceObjectID = "");

		void InitTerrainEditTools(FETerrain* Terrain);

		std::string FreeObjectName(FE_OBJECT_TYPE ObjectType);

		GLint MaxColorAttachments = 1;

		void FillTerrainLayerMaskWithRawData(const unsigned char* RawData, const FETerrain* Terrain, size_t LayerIndex);
		void CreateMaterialsFromOBJData(std::vector<FEObject*>& ResultArray);

		static void LoadTextureFileAsyncCallBack(void* OutputData);

		std::string EngineFolder = std::string(ENGINE_FOLDER) + "/";
		std::string ResourcesFolder = EngineFolder + "/Resources/";
	};

	#define RESOURCE_MANAGER FEResourceManager::getInstance()
}