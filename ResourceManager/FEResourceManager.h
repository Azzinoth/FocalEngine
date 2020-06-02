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
		FETexture* LoadPngTexture(const char* fileName, std::string Name = "", const char* saveFETexureTo = nullptr);
		FETexture* LoadPngTextureAndCompress(const char* fileName, bool usingAlpha, std::string Name = "");
		FETexture* LoadFETexture(const char* fileName, std::string Name = "");
		FETexture* LoadFETexture_(const char* fileName, std::string Name = "");
		
		void saveFETexture(const char* fileName, FETexture* texture, char* textureData);
		void saveFETexture(const char* fileName, int width, int height, char* textureData);
		void saveFETexture(const char* fileName, FETexture* texture);
		void deleteFETexture(FETexture* texture);
		std::vector<std::string> getTextureList();
		FETexture* getTexture(std::string name);
		bool setTextureName(FETexture* Texture, std::string TextureName);
		FETexture* noTexture;

		FEMesh* rawDataToMesh(std::vector<float>& positions);
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals);
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index);
		FEMesh* rawDataToMesh(float* positions, int posSize, float* UV, int UVSize, float* normals, int normSize, float* tangents, int tanSize, int* indices, int indexSize);
		FEMesh* rawObjDataToMesh();
		FEMesh* getSimpleMesh(std::string meshName);
		bool setMeshName(FEMesh* Mesh, std::string MeshName);

		std::vector<std::string> getMaterialList();
		FEMaterial* getMaterial(std::string name);
		FEMaterial* createMaterial(std::string Name = "");
		bool setMaterialName(FEMaterial* Material, std::string MaterialName);
		
		std::vector<std::string> getMeshList();
		FEMesh* getMesh(std::string name);
		FEMesh* LoadOBJMesh(const char* fileName, std::string Name = "", const char* saveFEMeshTo = nullptr);
		FEMesh* LoadFEMesh(const char* fileName, std::string Name = "");
		void saveFEMesh(const char* fileName);

		void clear();
		void loadStandardMeshes();
		void loadStandardMaterial();

		/*void FEResourceManager::addFEMeshToFile(std::fstream& file, FEMesh* Mesh);
		void saveAssets(const char* fileName);*/
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)

		FETexture* createTexture(std::string Name = "");
		FEMesh* FEResourceManager::createMesh(GLuint VaoID, unsigned int VertexCount, int VertexBuffersTypes, FEAABB AABB, std::string Name = "");

		FEPostProcess* createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name);
		std::unordered_map<std::string, FETexture*> textures;
		std::unordered_map<std::string, FEMaterial*> materials;
		std::unordered_map<std::string, FEMesh*> meshes;

		std::string getFileNameFromFilePath(std::string filePath);
		FEEntity* createEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name);

		void LoadFETexture_(const char* fileName, FETexture* existingTexture);
	};
}