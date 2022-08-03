#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class FEEditorSelectedObject;
class FEEditor;

class FEEditorHaloSelectionEffect
{
	friend FEEditorSelectedObject;
	friend FEEditor;
private:
	SINGLETON_PUBLIC_PART(FEEditorHaloSelectionEffect)
	SINGLETON_PRIVATE_PART(FEEditorHaloSelectionEffect)

	void InitializeResources();
	void ReInitializeResources();

	FEFramebuffer* HaloObjectsFb = nullptr;
	FEMaterial* HaloMaterial = nullptr;
	FEPostProcess* PostProcess = nullptr;

	FEShader* HaloDrawInstancedObjectShader = nullptr;
	FEShader* HaloDrawObjectShader = nullptr;
	FEShader* HaloFinalShader = nullptr;
};

#define HALO_SELECTION_EFFECT FEEditorHaloSelectionEffect::getInstance()