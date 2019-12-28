#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/FEScene.h"

namespace FocalEngine
{
	class FERenderer
	{
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void render(FEBasicCamera* currentCamera);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
	};
}

#endif FERENDERER_H