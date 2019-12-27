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
		FEMesh* rawObjDataToMesh();
		FEMesh* getSimpleMesh(std::string meshName);

		FEMesh* loadObjMeshData(const char* fileName);
	private:
		SINGLETON_PRIVATE_PART(FEResourceManager)
		FEMesh* cube;
	};
}