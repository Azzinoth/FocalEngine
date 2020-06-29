#include "FEGameModel.h"
using namespace FocalEngine;

FEGameModel::FEGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name)
{
	mesh = Mesh;
	material = Material;
	name = Name;
}

FEGameModel::~FEGameModel()
{
}

std::string FEGameModel::getName()
{
	return name;
}

void FEGameModel::setName(std::string newName)
{
	name = newName;
}