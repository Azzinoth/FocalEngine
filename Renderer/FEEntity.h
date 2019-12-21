#pragma once

#ifndef FEENTITY_H
#define FEENTITY_H

#include "FEMesh.h"
#include "FEMaterial.h"

namespace FocalEngine
{
	class FEEntity
	{
		friend FERenderer;
	public:
		FEEntity(FEMesh* Mesh, FEMaterial* Material);
		~FEEntity();

		FEMesh* mesh = nullptr; // need some basic mesh ?
		FEMaterial* material = nullptr; // need some basic material
		void setPosition(glm::vec3 newPosition);
		void render();
	private:
		glm::vec3 position = glm::vec3();
		glm::vec3 rotation = glm::vec3();
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

		glm::mat4 transformMatrix;
	};
}

#endif FEENTITY_H