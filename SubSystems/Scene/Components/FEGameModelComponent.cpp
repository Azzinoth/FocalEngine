#include "FEGameModelComponent.h"
using namespace FocalEngine;

FEGameModelComponent::FEGameModelComponent(FEGameModel* GameModelToSet)
{
	GameModel = GameModelToSet;
}

FEGameModelComponent::FEGameModelComponent(const FEGameModelComponent& Other)
{
	GameModel = Other.GameModel;
	bVisible = Other.bVisible;
	bCastShadows = Other.bCastShadows;
	bReceiveShadows = Other.bReceiveShadows;
	bUniformLighting = Other.bUniformLighting;
	bApplyPostprocess = Other.bApplyPostprocess;
	bWireframeMode = Other.bWireframeMode;
}

void FEGameModelComponent::operator=(const FEGameModelComponent& Other)
{
	GameModel = Other.GameModel;
	bVisible = Other.bVisible;
	bCastShadows = Other.bCastShadows;
	bReceiveShadows = Other.bReceiveShadows;
	bUniformLighting = Other.bUniformLighting;
	bApplyPostprocess = Other.bApplyPostprocess;
	bWireframeMode = Other.bWireframeMode;
}

bool FEGameModelComponent::IsVisible() const
{
	return bVisible;
}

void FEGameModelComponent::SetVisibility(bool NewValue)
{
	bVisible = NewValue;
}

bool FEGameModelComponent::IsCastShadows() const
{
	return bCastShadows;
}

void FEGameModelComponent::SetCastShadows(bool NewValue)
{
	bCastShadows = NewValue;
}

bool FEGameModelComponent::IsReceivingShadows() const
{
	return bReceiveShadows;
}

void FEGameModelComponent::SetReceivingShadows(bool NewValue)
{
	bReceiveShadows = NewValue;
}

bool FEGameModelComponent::IsUniformLighting() const
{
	return bUniformLighting;
}

void FEGameModelComponent::SetUniformLighting(bool NewValue)
{
	bUniformLighting = NewValue;
}

bool FEGameModelComponent::IsPostprocessApplied() const
{
	return bApplyPostprocess;
}

void FEGameModelComponent::SetIsPostprocessApplied(bool NewValue)
{
	bApplyPostprocess = NewValue;
}

bool FEGameModelComponent::IsWireframeMode() const
{
	return bWireframeMode;
}

void FEGameModelComponent::SetWireframeMode(bool NewValue)
{
	bWireframeMode = NewValue;
}

FEGameModel* FEGameModelComponent::GetGameModel()
{
	return GameModel;
}

void FEGameModelComponent::SetGameModel(FEGameModel* NewGameModel)
{
	GameModel = NewGameModel;
}