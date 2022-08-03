#pragma once

#ifndef FEGAMEMODEL_H
#define FEGAMEMODEL_H

#include "FEMesh.h"
#include "FEFramebuffer.h"

namespace FocalEngine
{
	enum FE_CULING_TYPE
	{
		FE_CULLING_NONE = 0,
		FE_CULLING_LODS = 1,
	};

	struct FELODRecord
	{
		FEMesh* Mesh = nullptr;
		float MaxDrawDistance = 0.0f;
		bool bBillboard = false;
		FEMaterial* Material = nullptr;
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

		FEMesh* Mesh = nullptr;
		FEMaterial* Material = nullptr;

		bool IsUsingLOD();
		void SetUsingLOD(bool NewValue);

		FEMaterial* GetMaterial();
		FEMaterial* GetBillboardMaterial();

		void SetMaterial(FEMaterial* NewValue);
		void SetBillboardMaterial(FEMaterial* NewValue);

		FEMesh* GetMesh();
		void SetMesh(FEMesh* NewValue);

		FEMesh* GetLODMesh(size_t LODIndex);
		void SetLODMesh(size_t LODIndex, FEMesh* NewValue);

		float GetLODMaxDrawDistance(size_t LODIndex);
		void SetLODMaxDrawDistance(size_t LODIndex, float NewValue);

		bool IsLODBillboard(size_t LODIndex);
		void SetIsLODBillboard(size_t LODIndex, bool NewValue);

		size_t GetMaxLODCount();
		size_t GetLODCount();

		float GetBillboardZeroRotaion();
		void SetBillboardZeroRotaion(float NewValue);

		float GetCullDistance();
		void SetCullDistance(float NewValue);

		float GetScaleFactor();
		void SetScaleFactor(float NewValue);
	private:
		int MaxLODCount = 4;
		float CullDistance = 300.0f;
		bool bHaveLODLevels = false;

		FEMaterial* BillboardMaterial = nullptr;
		std::vector<FELODRecord> LODRecords;
		float BillboardZeroRotaion = 0.0f;
		float ScaleFactor = 1.0f;

		void CorrectLODDistances();
	};
}

#endif FEGAMEMODEL_H