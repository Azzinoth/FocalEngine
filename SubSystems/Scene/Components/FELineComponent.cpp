#include "FELineComponent.h"
using namespace FocalEngine;

FELineComponent::FELineComponent(FELineCollection* LineCollectionToSet)
{
	LineCollection = LineCollectionToSet;
}

FELineCollection* FELineComponent::GetLineCollection() const
{
	return LineCollection;
}
void FELineComponent::SetLineCollection(FELineCollection* NewLineCollection)
{
	LineCollection = NewLineCollection;
}