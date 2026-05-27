#include "FELine.h"
using namespace FocalEngine;

FELine::FELine(glm::vec3 StartPoint, glm::vec3 EndPoint, glm::vec3 LineColor, float LineWidth)
{
	this->StartPoint = StartPoint;
	this->EndPoint = EndPoint;
	this->Color = LineColor;
	this->Width = LineWidth;
}