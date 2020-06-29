#pragma once

#ifndef FEGAMEMODEL_H
#define FEGAMEMODEL_H

#include "FEMesh.h"
#include "FEFramebuffer.h"

namespace FocalEngine
{
	class FEGameModel
	{
		friend FERenderer;
	public:
		FEGameModel(FEMesh* Mesh, FEMaterial* Material, std::string Name);
		~FEGameModel();

		FEMesh* mesh = nullptr;
		FEMaterial* material = nullptr;

		std::string getName();
		void setName(std::string newName);
	private:
		std::string name;
	};
}

#endif FEGAMEMODEL_H