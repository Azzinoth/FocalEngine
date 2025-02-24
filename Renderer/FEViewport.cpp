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

FEViewportType FEViewport::GetType() const
{
	return Type;
}

int FEViewport::GetX() const
{
	return X;
}

int FEViewport::GetY() const
{
	return Y;
}

void FEViewport::SetWidth(const int NewWidth)
{
	Width = NewWidth;
}

void FEViewport::SetHeight(const int NewHeight)
{
	Height = NewHeight;
}

int FEViewport::GetWidth() const
{
	return Width;
}

int FEViewport::GetHeight() const
{
	return Height;
}