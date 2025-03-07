#pragma once

#ifndef FELINE_H
#define FELINE_H

#include "../Core/FEObject.h"

namespace FocalEngine
{
	struct FELine
	{
		glm::vec3 Begin;
		glm::vec3 End;
		glm::vec3 Color;
		float Width;
	};
}

#endif FELINE_H