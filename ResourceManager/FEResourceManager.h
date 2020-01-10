#pragma once

#include "../CoreExtensions/PostProcessEffects/FEGammaAndHDRCorrection.h"
#include "../ResourceManager/FEObjLoader.h"
#include "../ThirdParty/lodepng.h"

namespace FocalEngine
{
	class FEngine;
	class FEScene;

	class FEResourceManager
	{
		friend FEngine;
		friend FEScene;
	public:
		SINGLETON_PUBLIC_PART(FEResourceManager)

		// to-do: add flags
		FETexture* createTexture(const char* file_name, std::string Name = "");
		// to-do: put creating of framebuffers here!

		FEMesh* rawDataToMesh(std::vector<float>& positions);
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals);
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index);
		FEMesh* rawObjDataToMesh();
		FEMesh* getSimpleMesh(std::string meshName);

		FEMesh* loadObjMeshData(const char* fileName);

		std::vector<std::string> getMaterialList();
		FEMaterial* getMaterial(std::string name);
		FEMaterial* createMaterial(std::string Name = "");
		FEEntity* createEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name);
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)
		FEMesh* plane;
		FEMesh* cube;

		FEPostProcess* createPostProcess(int ScreenWidth, int ScreenHeight, std::string Name);
		std::unordered_map<std::string, FEMaterial*> materials;

		std::string getFileNameFromFilePath(std::string filePath);
	};
}