#include "FETagComponent.h"
using namespace FocalEngine;

FETagComponent::FETagComponent(std::string TagToSet)
{
	Tag = TagToSet;
}

std::string FETagComponent::GetTag() const
{ 
	return Tag;
}

void FETagComponent::SetTag(std::string NewTag)
{ 
	Tag = NewTag;
}