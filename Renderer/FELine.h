#pragma once

#ifndef FELINE_H
#define FELINE_H

#include "../SubSystems/FETransformComponent.h"

namespace FocalEngine
{
	struct FELine
	{
		glm::vec3 begin;
		glm::vec3 end;
		glm::vec3 color;
		float width;
	};
}

#endif FELINE_H