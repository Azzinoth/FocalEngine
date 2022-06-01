#pragma once

#include "../Renderer/FEPostProcess.h"
#include "../Renderer/FETerrain.h"
#include "../ThirdParty/lodepng/lodepng.h"
#include "../SubSystems/FEJobManager.h"
#include "../ThirdParty/stb_image/stb_image.h"
#include "FEglTFLoader.h"

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

		FEShader* createShader(std::string shaderName, const char* vertexText, const char* fragmentText,
							   const char* tessControlText = nullptr, const char* tessEvalText = nullptr,
							   const char* geometryText = nullptr, const char* computeText = nullptr, std::string forceObjectID = "");

		bool makeShaderStandard(FEShader* shader);
		FEShader* getShader(std::string shaderID);
		std::vector<FEShader*> getShaderByName(std::string Name);
		std::vector<std::string> getShadersList();
		std::vector<std::string> getStandardShadersList();
		void deleteShader(FEShader* shader);
		bool replaceShader(std::string oldShaderID, FEShader* newShader);
		std::string loadGLSL(const char* fileName);

		FETexture* LoadPNGTexture(const char* fileName, std::string Name = "");
		FETexture* LoadJPGTexture(const char* fileName, std::string Name = "");
		FETexture* LoadBMPTexture(const char* fileName, std::string Name = "");
		FETexture* LoadFETexture(const char* fileName, std::string Name = "", FETexture* existingTexture = nullptr);
		FETexture* LoadFETextureUnmanaged(const char* fileName, std::string Name = "");
		FETexture* LoadFETexture(char* fileData, std::string Name = "", FETexture* existingTexture = nullptr);
		FETexture* LoadFETextureAsync(const char* fileName, std::string Name = "", FETexture* existingTexture = nullptr, std::string forceObjectID = "");
		FETexture* LoadPNGHeightmap(const char* fileName, FETerrain* terrain, std::string Name = "");
		FETexture* LoadFEHeightmap(const char* fileName, FETerrain* terrain, std::string Name = "");
		FETexture* rawDataToFETexture(unsigned char* textureData, int width, int height, GLint internalformat = -1, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);
		std::vector<FETexture*> channelsToFETextures(FETexture* sourceTexture);
		unsigned char* resizeTextureRawData(FETexture* sourceTexture, size_t targetWidth, size_t targetHeight, int filtrationLevel = 0);
		unsigned char* resizeTextureRawData(unsigned char* textureData, size_t width, size_t height, size_t targetWidth, size_t targetHeight, GLint internalFormat, int filtrationLevel = 0);
		void resizeTexture(FETexture* sourceTexture, int targetWidth, int targetHeight, int filtrationLevel = 0);
		FETexture* createTextureWithTransparency(FETexture* originalTexture, FETexture* maskTexture);

		void saveFETexture(FETexture* texture, const char* fileName);
		bool exportFETextureToPNG(FETexture* textureToExport, const char* fileName);
		bool exportRawDataToPNG(const char* fileName, unsigned char* textureData, int width, int height, GLint internalformat);
		void deleteFETexture(FETexture* texture);
		std::vector<std::string> getTextureList();
		FETexture* getTexture(std::string ID);
		std::vector<FETexture*> getTextureByName(std::string Name);
		bool makeTextureStandard(FETexture* texture);
		FETexture* noTexture;
		FETexture* createTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool unManaged = true, std::string Name = "");
		FETexture* createSameFormatTexture(FETexture* exampleTexture, int differentW = 0, int differentH = 0, bool unManaged = true, std::string Name = "");
		void updateAsyncLoadedResources();

		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index, std::string Name = "");
		FEMesh* rawDataToMesh(float* positions, int posSize,
							  float* UV, int UVSize,
							  float* normals, int normSize,
							  float* tangents, int tanSize,
							  int* indices, int indexSize,
							  float* matIndexs = nullptr, int matIndexsSize = 0, int matCount = 0,
							  std::string Name = "");

		void deleteFEMesh(FEMesh* mesh);
		bool makeMeshStandard(FEMesh* mesh);

		std::vector<std::string> getMeshList();
		std::vector<std::string> getStandardMeshList();
		FEMesh* getMesh(std::string ID);
		std::vector<FEMesh*> getMeshByName(std::string Name);
		std::vector<FEObject*> importOBJ(const char* fileName, bool forceOneMesh = false);
		FEMesh* loadFEMesh(const char* fileName, std::string Name = "");
		void saveFEMesh(FEMesh* Mesh, const char* fileName);

		FEFramebuffer* createFramebuffer(int attachments, int Width, int Height, bool HDR = true);

		std::vector<std::string> getMaterialList();
		std::vector<std::string> getStandardMaterialList();
		FEMaterial* getMaterial(std::string ID);
		std::vector<FEMaterial*> getMaterialByName(std::string Name);
		FEMaterial* createMaterial(std::string Name = "", std::string forceObjectID = "");
		bool makeMaterialStandard(FEMaterial* material);
		void deleteMaterial(FEMaterial* Material);

		std::vector<std::string> getGameModelList();
		std::vector<std::string> getStandardGameModelList();
		FEGameModel* getGameModel(std::string ID);
		std::vector<FEGameModel*> getGameModelByName(std::string Name);
		FEGameModel* createGameModel(FEMesh* Mesh = nullptr, FEMaterial* Material = nullptr, std::string Name = "", std::string forceObjectID = "");
		bool makeGameModelStandard(FEGameModel* gameModel);
		void deleteGameModel(FEGameModel* gameModel);

		std::vector<std::string> getPrefabList();
		std::vector<std::string> getStandardPrefabList();
		FEPrefab* getPrefab(std::string ID);
		std::vector<FEPrefab*> getPrefabByName(std::string Name);
		FEPrefab* createPrefab(FEGameModel* gameModel = nullptr, std::string Name = "", std::string forceObjectID = "");
		bool makePrefabStandard(FEPrefab* prefab);
		void deletePrefab(FEPrefab* prefab);

		FETerrain* createTerrain(bool createHeightMap = true, std::string name = "", std::string forceObjectID = "");
		void activateTerrainVacantLayerSlot(FETerrain* terrain, FEMaterial* material);
		void loadTerrainLayerMask(const char* fileName, FETerrain* terrain, size_t layerIndex);
		void saveTerrainLayerMask(const char* fileName, FETerrain* terrain, size_t layerIndex);
		void fillTerrainLayerMask(FETerrain* terrain, size_t layerIndex);
		void clearTerrainLayerMask(FETerrain* terrain, size_t layerIndex);
		void deleteTerrainLayerMask(FETerrain* terrain, size_t layerIndex);

		void clear();
		void loadStandardMeshes();
		void loadStandardMaterial();
		void loadStandardGameModels();
		void loadStandardPrefabs();

		void reSaveStandardTextures();
		void reSaveStandardMeshes();

		std::string getDefaultResourcesFolder();
		std::vector<FEObject*> LoadGLTF(const char* fileName);

		std::vector<FEObject*> importAsset(const char* fileName);
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		std::unordered_map<std::string, FEShader*> shaders;
		std::unordered_map<std::string, FEShader*> standardShaders;

		FETexture* createTexture(std::string Name = "", std::string forceObjectID = "");
		FEMesh* FEResourceManager::createMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name = "");

		FEPostProcess* createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name);
		std::unordered_map<std::string, FETexture*> textures;
		std::unordered_map<std::string, FETexture*> standardTextures;

		std::unordered_map<std::string, FEMaterial*> materials;
		std::unordered_map<std::string, FEMaterial*> standardMaterials;

		std::unordered_map<std::string, FEMesh*> meshes;
		std::unordered_map<std::string, FEMesh*> standardMeshes;

		std::unordered_map<std::string, FEGameModel*> gameModels;
		std::unordered_map<std::string, FEGameModel*> standardGameModels;

		std::unordered_map<std::string, FEPrefab*> prefabs;
		std::unordered_map<std::string, FEPrefab*> standardPrefabs;

		std::string getFileNameFromFilePath(std::string filePath);
		FEEntity* createEntity(FEGameModel* gameModel, std::string Name, std::string forceObjectID = "");
		FEEntity* createEntity(FEPrefab* prefab, std::string Name, std::string forceObjectID = "");

		void initTerrainEditTools(FETerrain* terrain);

		std::string freeObjectName(FEObjectType objectType);

		GLint maxColorAttachments = 1;

		std::string defaultResourcesFolder = "Resources//";
		void fillTerrainLayerMaskWithRawData(unsigned char* rawData, FETerrain* terrain, size_t layerIndex);
		void createMaterialsFromOBJData(std::vector<FEObject*>& resultArray);
	};

	#define RESOURCE_MANAGER FEResourceManager::getInstance()
}