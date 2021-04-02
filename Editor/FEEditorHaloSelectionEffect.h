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

	void initializeResources();
	void reInitializeResources();

	FEFramebuffer* haloObjectsFB = nullptr;
	FEMaterial* haloMaterial = nullptr;
	FEPostProcess* postProcess = nullptr;

	FEShader* HaloDrawInstancedObjectShader = nullptr;
	FEShader* HaloDrawObjectShader = nullptr;
	FEShader* HaloFinalShader = nullptr;
};

#define HALO_SELECTION_EFFECT FEEditorHaloSelectionEffect::getInstance()