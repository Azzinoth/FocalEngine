#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "FEEntity.h"

namespace FocalEngine
{
	const float FOV = 70.0f;
	const float NEAR_PLANE = 0.1f;
	const float FAR_PLANE = 5000.0f;

	class FERenderer
	{
	public:
		static FERenderer& getInstance()
		{
			static FERenderer Renderer;
			return Renderer;
		}
		glm::mat4 getProjectionMatrix();

		void addToScene(FEEntity* newEntity);
		void render();
	private:
		FERenderer();
		std::vector<FEEntity*> sceneGraph;
	};
}

#endif FERENDERER_H