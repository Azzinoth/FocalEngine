#include "FEGameModel.h"
using namespace FocalEngine;

FEGameModel::FEGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name) : FEAsset(FE_GAMEMODEL, Name)
{
	mesh = Mesh;
	material = Material;
	setName(Name);
}

FEGameModel::~FEGameModel()
{
}