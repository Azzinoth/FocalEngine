#pragma once

#include "../CoreExtensions/PostProcessEffects/FEFXAA.h"
#include "../SubSystems/FETerrain.h"
#include "../CoreExtensions/StandardMaterial/TerrainMaterial/FETerrainShader.h"
#include "../ResourceManager/FEObjLoader.h"
#include "../ThirdParty/lodepng/lodepng.h"

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
							   const char* geometryText = nullptr, const char* computeText = nullptr);

		bool makeShaderStandard(FEShader* shader);
		bool setShaderName(FEShader* shader, std::string shaderName);
		FEShader* getShader(std::string shaderName);
		std::vector<std::string> getShadersList();
		std::vector<std::string> getStandardShadersList();
		void deleteShader(std::string shaderName);
		bool replaceShader(std::string oldShaderName, FEShader* newShader);
		std::string loadGLSL(const char* fileName);

		// to-do: add flags
		FETexture* LoadPngTextureAndCompress(const char* fileName, bool usingAlpha, std::string Name = "");
		FETexture* LoadPngTextureWithTransparencyMaskAndCompress(const char* mainfileName, const char* maskFileName, std::string Name);
		FETexture* LoadFETexture(const char* fileName, std::string Name = "");
		FETexture* LoadFETextureStandAlone(const char* fileName, std::string Name = "");
		FETexture* LoadHeightmap(const char* fileName, std::string Name = "");
		
		void saveFETexture(const char* fileName, FETexture* texture);
		void saveFETexture(const char* fileName, char* textureData, int width, int height, bool isAlphaUsed = false);
		void deleteFETexture(FETexture* texture);
		std::vector<std::string> getTextureList();
		FETexture* getTexture(std::string name);
		bool makeTextureStandard(FETexture* texture);
		bool setTextureName(FETexture* Texture, std::string TextureName);
		FETexture* noTexture;

		FEMesh* rawDataToMesh(std::vector<float>& positions, std::string Name = "");
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::string Name = "");
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index, std::string Name = "");
		FEMesh* rawDataToMesh(float* positions, int posSize, float* UV, int UVSize, float* normals, int normSize, float* tangents, int tanSize, int* indices, int indexSize, std::string Name = "");
		FEMesh* rawObjDataToMesh();
		void deleteFEMesh(FEMesh* mesh);
		bool makeMeshStandard(FEMesh* mesh);
		bool setMeshName(FEMesh* Mesh, std::string MeshName);

		std::vector<std::string> getMaterialList();
		std::vector<std::string> getStandardMaterialList();
		FEMaterial* getMaterial(std::string name);
		FEMaterial* createMaterial(std::string Name = "");
		bool makeMaterialStandard(FEMaterial* material);
		bool setMaterialName(FEMaterial* Material, std::string MaterialName);
		
		std::vector<std::string> getMeshList();
		std::vector<std::string> getStandardMeshList();
		FEMesh* getMesh(std::string meshName);
		FEMesh* LoadOBJMesh(const char* fileName, std::string Name = "", const char* saveFEMeshTo = nullptr);
		FEMesh* LoadFEMesh(const char* fileName, std::string Name = "");
		void saveFEMesh(const char* fileName);

		std::vector<std::string> getGameModelList();
		std::vector<std::string> getStandardGameModelList();
		FEGameModel* getGameModel(std::string name);
		FEGameModel* createGameModel(FEMesh* Mesh = nullptr, FEMaterial* Material = nullptr, std::string Name = "");
		bool makeGameModelStandard(FEGameModel* gameModel);
		bool setGameModelName(FEGameModel* gameModel, std::string gameModelName);
		void deleteGameModel(FEGameModel* gameModel);

		FETerrain* createTerrain(std::string name);
		FETerrain* getTerrain(std::string terrainName);
		bool setTerrainName(FETerrain* terrain, std::string terrainName);
		std::vector<std::string> getTerrainList();

		void clear();
		void loadStandardMeshes();
		void loadStandardMaterial();
		void loadStandardGameModels();
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		std::unordered_map<std::string, FEShader*> shaders;
		std::unordered_map<std::string, FEShader*> standardShaders;
		void initializeShaderBlueprints(std::vector<FEShaderBlueprint>& list);

		FETexture* createTexture(std::string Name = "");
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

		std::unordered_map<std::string, FETerrain*> terrains;

		std::string getFileNameFromFilePath(std::string filePath);
		FEEntity* createEntity(FEGameModel* gameModel, std::string Name);

		void LoadFETexture(const char* fileName, FETexture* existingTexture);
	};
}