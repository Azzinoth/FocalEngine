#pragma once

#ifndef FEGAMEMODEL_H
#define FEGAMEMODEL_H

#include "FEMesh.h"
#include "FEFramebuffer.h"

namespace FocalEngine
{
	enum FECulingType
	{
		FE_CULLING_NONE = 0,
		FE_CULLING_LODS = 1,
	};

	struct FELODRecord
	{
		FEMesh* mesh = nullptr;
		float maxDrawDistance = 0.0f;
		bool isBillboard = false;
		FEMaterial* material = nullptr;
	};

	class FEPrefab;

	class FEGameModel : public FEObject
	{
		friend FERenderer;
		friend FEPrefab;
		friend FEEntityInstanced;
	public:
		FEGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name);
		~FEGameModel();

		FEMesh* mesh = nullptr;
		FEMaterial* material = nullptr;

		bool useLODlevels();
		void setUsingLODlevels(bool newValue);

		FEMaterial* getMaterial();
		FEMaterial* getBillboardMaterial();

		void setMaterial(FEMaterial* newValue);
		void setBillboardMaterial(FEMaterial* newValue);

		FEMesh* getMesh();
		void setMesh(FEMesh* newValue);

		FEMesh* getLODMesh(size_t LODindex);
		void setLODMesh(size_t LODindex, FEMesh* newValue);

		float getLODMaxDrawDistance(size_t LODindex);
		void setLODMaxDrawDistance(size_t LODindex, float newValue);

		bool isLODBillboard(size_t LODindex);
		void setIsLODBillboard(size_t LODindex, bool newValue);

		size_t getMaxLODCount();
		size_t getLODCount();

		float getBillboardZeroRotaion();
		void setBillboardZeroRotaion(float newValue);

		float getCullDistance();
		void setCullDistance(float newValue);

		float getScaleFactor();
		void setScaleFactor(float newValue);

		bool dirtyFlag = false;
	private:
		int maxLODCount = 4;
		float cullDistance = 300.0f;
		bool haveLODlevels = false;

		FEMaterial* billboardMaterial = nullptr;
		std::vector<FELODRecord> LODRecords;
		float billboardZeroRotaion = 0.0f;
		float scaleFactor = 1.0f;

		void correctLODDistances();
	};
}

#endif FEGAMEMODEL_H