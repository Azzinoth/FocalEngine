#pragma once

#ifndef FEENTITY_H
#define FEENTITY_H

#include "FEMesh.h"
#include "FEMaterial.h"
#include "../CoreExtensions/StandardMaterial/FEStandardMaterial.h"

namespace FocalEngine
{
	class FEEntity
	{
		friend FERenderer;
	public:
		FEEntity(FEMesh* Mesh, FEMaterial* Material);
		FEEntity(FEMesh* Mesh);
		~FEEntity();

		FEMesh* mesh = nullptr; // need some basic mesh ?
		FEMaterial* material = nullptr; // need some basic material

		glm::vec3 getPosition();
		glm::vec3 getRotation();
		glm::vec3 getScale();

		void setPosition(glm::vec3 newPosition);
		void setRotation(glm::vec3 newRotation);
		void setScale(glm::vec3 newScale);

		void render();
	private:
		glm::vec3 position = glm::vec3();
		glm::vec3 rotation = glm::vec3();
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

		void updateWorldMatrix();
		glm::mat4 worldMatrix;
	};
}

#endif FEENTITY_H