#pragma once

#ifndef FEENTITY_H
#define FEENTITY_H

#include "FEMesh.h"
#include "FEFramebuffer.h"

namespace FocalEngine
{
	class FEEntity
	{
		friend FERenderer;
	public:
		FEEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name);
		~FEEntity();

		FEMesh* mesh = nullptr;
		FEMaterial* material = nullptr;

		glm::vec3 getPosition();
		glm::vec3 getRotation();
		glm::vec3 getScale();

		void setPosition(glm::vec3 newPosition);
		void setRotation(glm::vec3 newRotation);
		void setScale(glm::vec3 newScale);

		void render();

		bool isVisible();
		void setVisibility(bool isVisible);

		std::string getName();
		void setName(std::string newName);
	private:
		std::string name;

		glm::vec3 position = glm::vec3();
		glm::vec3 rotation = glm::vec3();
		glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

		void updateWorldMatrix();
		glm::mat4 worldMatrix;

		bool visible = true;
	};
}

#endif FEENTITY_H