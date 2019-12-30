#pragma once

#include "../Renderer/FEEntity.h"
#include "../ResourceManager/FEObjLoader.h"

namespace FocalEngine
{
	class FEResourceManager
	{
	public:
		SINGLETON_PUBLIC_PART(FEResourceManager)

		FEMesh* rawDataToMesh(std::vector<float>& positions);
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals);
		FEMesh* rawDataToMesh(std::vector<float>& positions, std::vector<float>& normals, std::vector<float>& tangents, std::vector<float>& UV, std::vector<int>& index);
		FEMesh* rawObjDataToMesh();
		FEMesh* getSimpleMesh(std::string meshName);

		FEMesh* loadObjMeshData(const char* fileName);
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)
		FEMesh* plane;
		FEMesh* cube;
	};
}