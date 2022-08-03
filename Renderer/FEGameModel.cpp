#include "FEGameModel.h"
using namespace FocalEngine;

FEGameModel::FEGameModel(FEMesh* Mesh, FEMaterial* Material, const std::string Name) : FEObject(FE_GAMEMODEL, Name)
{
	this->Mesh = Mesh;
	this->Material = Material;
	SetName(Name);
	LODRecords.resize(4);

	LODRecords[0].MaxDrawDistance = 50.0f;
	LODRecords[1].MaxDrawDistance = 150.0f;
	LODRecords[2].MaxDrawDistance = 200.0f;
	LODRecords[3].MaxDrawDistance = CullDistance;
}

FEGameModel::~FEGameModel()
{
}

bool FEGameModel::IsUsingLOD()
{
	return bHaveLODLevels;
}

void FEGameModel::SetUsingLOD(const bool NewValue)
{
	if (bHaveLODLevels == NewValue)
		return;
	bHaveLODLevels = NewValue;

	for (size_t i = 1; i < LODRecords.size(); i++)
	{
		LODRecords[i].Mesh = nullptr;
	}
}

FEMesh* FEGameModel::GetLODMesh(const size_t LODIndex)
{
	if (LODIndex >= LODRecords.size())
		return nullptr;

	if (LODIndex == 0 && LODRecords[LODIndex].Mesh == nullptr)
		return Mesh;

	return LODRecords[LODIndex].Mesh;
}

void FEGameModel::SetLODMesh(const size_t LODIndex, FEMesh* NewValue)
{
	if (LODIndex >= LODRecords.size())
		return;

	if (LODIndex == 0)
		Mesh = NewValue;

	LODRecords[LODIndex].Mesh = NewValue;
	SetDirtyFlag(true);
}

size_t FEGameModel::GetLODCount()
{
	int RealLODCount = 0;
	for (size_t j = 0; j < LODRecords.size(); j++)
	{
		if (GetLODMesh(j) == nullptr)
			break;
		RealLODCount++;
	}

	return RealLODCount;
}

FEMesh* FEGameModel::GetMesh()
{
	if (this->bHaveLODLevels && LODRecords[0].Mesh != nullptr)
	{
		return LODRecords[0].Mesh;
	}
	else
	{
		return Mesh;
	}
}

void FEGameModel::SetMesh(FEMesh* NewValue)
{
	LODRecords[0].Mesh = NewValue;
	Mesh = NewValue;
}

float FEGameModel::GetLODMaxDrawDistance(const size_t LODIndex)
{
	if (LODIndex >= LODRecords.size())
		return 0.0f;

	return LODRecords[LODIndex].MaxDrawDistance;
}

void FEGameModel::SetLODMaxDrawDistance(const size_t LODIndex, const float NewValue)
{
	if (LODIndex >= LODRecords.size())
		return;

	LODRecords[LODIndex].MaxDrawDistance = NewValue;
	CorrectLODDistances();
}

bool FEGameModel::IsLODBillboard(const size_t LODIndex)
{
	if (LODIndex >= LODRecords.size())
		return false;

	return LODRecords[LODIndex].bBillboard;
}

void FEGameModel::SetIsLODBillboard(const size_t LODIndex, const bool NewValue)
{
	if (LODIndex >= LODRecords.size())
		return;

	if (NewValue && !LODRecords[LODIndex].bBillboard)
	{
		for (size_t i = 0; i < LODRecords.size(); i++)
		{
			LODRecords[LODIndex].bBillboard = false;
		}

		LODRecords[LODIndex].bBillboard = NewValue;
		LODRecords[LODIndex].MaxDrawDistance = CullDistance;

		for (size_t i = LODIndex + 1; i < LODRecords.size(); i++)
		{
			LODRecords[i].Mesh = nullptr;
			LODRecords[i].MaxDrawDistance = CullDistance + 10.0f;
		}
	}

	LODRecords[LODIndex].bBillboard = NewValue;
}

FEMaterial* FEGameModel::GetMaterial()
{
	return Material;
}

FEMaterial* FEGameModel::GetBillboardMaterial()
{
	return BillboardMaterial;
}

void FEGameModel::SetMaterial(FEMaterial* NewValue)
{
	Material = NewValue;
}

void FEGameModel::SetBillboardMaterial(FEMaterial* NewValue)
{
	BillboardMaterial = NewValue;
}

float FEGameModel::GetBillboardZeroRotaion()
{
	return BillboardZeroRotaion;
}

void FEGameModel::SetBillboardZeroRotaion(const float NewValue)
{
	BillboardZeroRotaion = NewValue;
}

float FEGameModel::GetCullDistance()
{
	return CullDistance;
}

void FEGameModel::SetCullDistance(float NewValue)
{
	if (NewValue < 0.1f)
		NewValue = 0.1f;

	CullDistance = NewValue;
	CorrectLODDistances();
}

size_t FEGameModel::GetMaxLODCount()
{
	return MaxLODCount;
}

float FEGameModel::GetScaleFactor()
{
	return ScaleFactor;
}

void FEGameModel::SetScaleFactor(const float NewValue)
{
	ScaleFactor = NewValue;
}

void FEGameModel::CorrectLODDistances()
{
	for (size_t i = 0; i < LODRecords.size(); i++)
	{
		if (LODRecords[i].Mesh == nullptr)
		{
			LODRecords[i].MaxDrawDistance = CullDistance + 1;
		}
	}
}