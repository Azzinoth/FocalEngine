#pragma once

#ifndef FEGAMEMODEL_H
#define FEGAMEMODEL_H

#include "FEMesh.h"
#include "FEFramebuffer.h"

namespace FocalEngine
{
	class FEGameModel : public FEAsset
	{
		friend FERenderer;
	public:
		FEGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name);
		~FEGameModel();

		FEMesh* mesh = nullptr;
		FEMaterial* material = nullptr;
	private:
	};
}

#endif FEGAMEMODEL_H