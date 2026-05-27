#include "FEVisibilityComponent.h"
using namespace FocalEngine;

bool FEVisibilityComponent::IsVisible(ComponentVisibilityType Type) const
{
    return (VisibleTypes & Type) != 0;
}

void FEVisibilityComponent::SetVisible(ComponentVisibilityType Type, bool bVisible)
{
    if (bVisible)
        VisibleTypes |= Type;
    else
        VisibleTypes &= ~Type;
}

void FEVisibilityComponent::SetVisible(bool bVisible)
{
    VisibleTypes = bVisible ? ALL : 0;
}