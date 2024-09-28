#include "FEViewport.h"
using namespace FocalEngine;

FEViewport::FEViewport()
{
	ID = APPLICATION.GetUniqueHexID();
}

std::string FEViewport::GetID() const
{
	return ID;
}

int FEViewport::GetX() const
{
	return X;
}

int FEViewport::GetY() const
{
	return Y;
}

int FEViewport::GetWidth() const
{
	return Width;
}

int FEViewport::GetHeight() const
{
	return Height;
}