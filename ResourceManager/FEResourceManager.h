#pragma once

#include "../Renderer/FEPostProcess.h"
#include "../SubSystems/FETerrain.h"
#include "../CoreExtensions/StandardMaterial/TerrainMaterial/FETerrainShader.h"
#include "../ResourceManager/FEObjLoader.h"
#include "../ThirdParty/lodepng/lodepng.h"
#include "../SubSystems/FEJobManager.h"

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
		FEShader* getShader(std::string shaderID);
		std::vector<std::string> getShadersList();
		std::vector<std::string> getStandardShadersList();
		void deleteShader(std::string shaderID);
		bool replaceShader(std::string oldShaderID, FEShader* newShader);
		std::string loadGLSL(const char* fileName);

		FETexture* LoadPNGTexture(const char* fileName, bool usingAlpha, std::string Name = "");
		FETexture* LoadPNGTextureWithTransparencyMask(const char* mainfileName, const char* maskFileName, std::string Name);
		FETexture* LoadFETexture(const char* fileName, std::string Name = "", FETexture* existingTexture = nullptr);
		FETexture* LoadFETextureUnmanaged(const char* fileName, std::string Name = "");
		FETexture* LoadFETexture(char* fileData, std::string Name = "", FETexture* existingTexture = nullptr);
		FETexture* LoadFETextureAsync(const char* fileName, std::string Name = "", FETexture* existingTexture = nullptr, std::string forceObjectID = "");
		FETexture* LoadPNGHeightmap(const char* fileName, FETerrain* terrain, std::string Name = "");
		FETexture* LoadFEHeightmap(const char* fileName, FETerrain* terrain, std::string Name = "");
		FETexture* rawDataToFETexture(char* textureData, int width, int height, bool isAlphaUsed = false);
		
		void saveFETexture(FETexture* texture, const char* fileName);
		void deleteFETexture(FETexture* texture);
		std::vector<std::string> getTextureList();
		FETexture* getTexture(std::string ID);
		bool makeTextureStandard(FETexture* texture);
		FETexture* noTexture;
		FETexture* createTexture(GLint InternalFormat, GLenum Format, int Width, int Height, bool unManaged = true, std::string Name = "");
		FETexture* createSameFormatTexture(FETexture* exampleTexture, int differentW = 0, int differentH = 0, bool unManaged = true, std::string Name = "");

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
		FEMesh* LoadOBJMesh(const char* fileName, std::string Name = "");
		FEMesh* LoadFEMesh(const char* fileName, std::string Name = "");
		void saveFEMesh(FEMesh* Mesh, const char* fileName);

		FEFramebuffer* createFramebuffer(int attachments, int Width, int Height, bool HDR = true);

		std::vector<std::string> getMaterialList();
		std::vector<std::string> getStandardMaterialList();
		FEMaterial* getMaterial(std::string ID);
		FEMaterial* createMaterial(std::string Name = "", std::string forceObjectID = "");
		bool makeMaterialStandard(FEMaterial* material);
		void deleteMaterial(FEMaterial* Material);

		std::vector<std::string> getGameModelList();
		std::vector<std::string> getStandardGameModelList();
		FEGameModel* getGameModel(std::string ID);
		FEGameModel* createGameModel(FEMesh* Mesh = nullptr, FEMaterial* Material = nullptr, std::string Name = "", std::string forceObjectID = "");
		bool makeGameModelStandard(FEGameModel* gameModel);
		void deleteGameModel(FEGameModel* gameModel);

		FETerrain* createTerrain(bool createHeightMap = true, std::string name = "", std::string forceObjectID = "");

		void clear();
		void loadStandardMeshes();
		void loadStandardMaterial();
		void loadStandardGameModels();

		void reSaveStandardTextures();
		void reSaveStandardMeshes();

		float totalTimeDisk = 0.0f;
		float TimeOpenGL = 0.0f;
		float TimeOpenGLmip = 0.0f;
		float TimeOpenGLmipload = 0.0f;
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		static const int defaultHeighttMapResolution = 1024;

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

		std::string getFileNameFromFilePath(std::string filePath);
		FEEntity* createEntity(FEGameModel* gameModel, std::string Name, std::string forceObjectID = "");

		void initTerrainEditTools(FETerrain* terrain);

		std::string freeObjectName(FEObjectType objectType);

		GLint maxColorAttachments = 1;
	};
}