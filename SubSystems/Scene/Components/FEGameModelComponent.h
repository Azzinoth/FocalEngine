#pragma once
#include "../Renderer/FEGameModel.h"

namespace FocalEngine
{
	struct FEGameModelComponent
	{
		FEGameModelComponent() {};
		FEGameModelComponent(FEGameModel* GameModelToSet);
		FEGameModelComponent(const FEGameModelComponent& Other);
		void operator=(const FEGameModelComponent& Other);

		bool IsVisible() const;
		void SetVisibility(bool NewValue);

		bool IsCastShadows() const;
		void SetCastShadows(bool NewValue);

		bool IsReceivingShadows() const;
		void SetReceivingShadows(bool NewValue);

		bool IsUniformLighting() const;
		void SetUniformLighting(bool NewValue);

		bool IsPostprocessApplied() const;
		void SetIsPostprocessApplied(bool NewValue);

		bool IsWireframeMode() const;
		void SetWireframeMode(bool NewValue);

		FEGameModel* GameModel = nullptr;
		
	private:
		bool bVisible = true;
		bool bCastShadows = true;
		bool bReceiveShadows = true;
		bool bUniformLighting = false;
		bool bApplyPostprocess = true;
		bool bWireframeMode = false;
	};
}