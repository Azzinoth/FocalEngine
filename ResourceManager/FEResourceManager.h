#pragma once

#include "../CoreExtensions/PostProcessEffects/FEGammaAndHDRCorrection.h"
#include "../ResourceManager/FEObjLoader.h"
#include "../ThirdParty/lodepng.h"

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

		// to-do: add flags
		FETexture* LoadPngTextureAndCompress(const char* fileName, bool usingAlpha, std::string Name = "");
		FETexture* LoadFETexture(const char* fileName, std::string Name = "");
		FETexture* LoadFETextureStandAlone(const char* fileName, std::string Name = "");
		
		void saveFETexture(const char* fileName, FETexture* texture);
		void saveFETexture(const char* fileName, char* textureData, int width, int height, bool isAlphaUsed = false);
		void deleteFETexture(FETexture* texture);
		std::vector<std::string> getTextureList();
		FETexture* getTexture(std::string name);
		bool setTextureName(FETexture* Texture, std::string TextureName);
		FETexture* noTexture;

		FEMesh* rawDataToMesh(std::vector<float>& positions, std::string Name = "");
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::string Name = "");
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index, std::string Name = "");
		FEMesh* rawDataToMesh(float* positions, int posSize, float* UV, int UVSize, float* normals, int normSize, float* tangents, int tanSize, int* indices, int indexSize, std::string Name = "");
		FEMesh* rawObjDataToMesh();
		void deleteFEMesh(FEMesh* mesh);
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

		void clear();
		void loadStandardMeshes();
		void loadStandardMaterial();
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		FETexture* createTexture(std::string Name = "");
		FEMesh* FEResourceManager::createMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name = "");

		FEPostProcess* createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name);
		std::unordered_map<std::string, FETexture*> textures;

		std::unordered_map<std::string, FEMaterial*> materials;
		std::unordered_map<std::string, FEMaterial*> standardMaterials;

		std::unordered_map<std::string, FEMesh*> meshes;
		std::unordered_map<std::string, FEMesh*> standardMeshes;

		std::string getFileNameFromFilePath(std::string filePath);
		FEEntity* createEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name);

		void LoadFETexture(const char* fileName, FETexture* existingTexture);
	};
}