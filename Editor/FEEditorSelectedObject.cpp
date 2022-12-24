#include "FEEditorSelectedObject.h"
using namespace FocalEngine;

FEEditorSelectedObject* FEEditorSelectedObject::Instance = nullptr;
FEEditorSelectedObject::FEEditorSelectedObject() {}
FEEditorSelectedObject::~FEEditorSelectedObject() {}

void FEEditorSelectedObject::InitializeResources()
{
	HALO_SELECTION_EFFECT.InitializeResources();
	
	PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());
	delete PixelAccurateSelectionFB->GetColorAttachment();
	PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight()));

	PixelAccurateSelectionMaterial = RESOURCE_MANAGER.CreateMaterial("pixelAccurateSelectionMaterial");
	RESOURCE_MANAGER.MakeMaterialStandard(PixelAccurateSelectionMaterial);

	FEPixelAccurateSelection = RESOURCE_MANAGER.CreateShader("FEPixelAccurateSelection", RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_PixelAccurateSelection_VS.glsl").c_str(),
																						 RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 nullptr,
																						 "4279660C7D3D27360358354E"/*"FEPixelAccurateSelection"*/);
	RESOURCE_MANAGER.MakeShaderStandard(FEPixelAccurateSelection);
	PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;

	FEPixelAccurateInstancedSelection = RESOURCE_MANAGER.CreateShader("FEPixelAccurateInstancedSelection", RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_PixelAccurateSelection_INSTANCED_VS.glsl").c_str(),
																										   RESOURCE_MANAGER.LoadGLSL("Editor//Materials//FE_PixelAccurateSelection_FS.glsl").c_str(),
																									       nullptr,
																										   nullptr,
																										   nullptr,
																										   nullptr,
																										   "0E213D3542135C15471F0D6B"/*"FEPixelAccurateInstancedSelection"*/);

	RESOURCE_MANAGER.MakeShaderStandard(FEPixelAccurateInstancedSelection);

	const FEShaderParam ColorParam(glm::vec3(0.0f, 0.0f, 0.0f), "baseColor");
	PixelAccurateSelectionMaterial->AddParameter(ColorParam);
}

void FEEditorSelectedObject::ReInitializeResources()
{
	HALO_SELECTION_EFFECT.ReInitializeResources();

	delete PixelAccurateSelectionFB;
	PixelAccurateSelectionFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());
	delete PixelAccurateSelectionFB->GetColorAttachment();
	PixelAccurateSelectionFB->SetColorAttachment(RESOURCE_MANAGER.CreateTexture(GL_RGB, GL_RGB, ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight()));
}

void FEEditorSelectedObject::SetOnUpdateFunc(void(*Func)())
{
	OnUpdateFunc = Func;
}

FEObject* FEEditorSelectedObject::GetSelected() const
{
	return Container;
}

bool FEEditorSelectedObject::GetDirtyFlag() const
{
	return bDirtyFlag;
}

void FEEditorSelectedObject::SetDirtyFlag(const bool NewValue)
{
	bDirtyFlag = NewValue;
}

void FEEditorSelectedObject::SetSelected(FEObject* SelectedObject)
{
	if (SelectedObject == nullptr)
		return;
	
	if (Container != SelectedObject)
		bDirtyFlag = true;

	if (Container != nullptr && Container->GetType() == FE_TERRAIN && Container != SelectedObject)
	{
		FETerrain* DeselectedTerrain = reinterpret_cast<FETerrain*>(Container);
		DeselectedTerrain->SetBrushMode(FE_TERRAIN_BRUSH_NONE);
	}

	Container = SelectedObject;
	if (OnUpdateFunc != nullptr)
		OnUpdateFunc();
}

void FEEditorSelectedObject::Clear()
{
	if (InstancedSubObjectIndexSelected != -1 && SCENE.GetEntityInstanced(Container->GetObjectID()) != nullptr)
	{
		SCENE.GetEntityInstanced(Container->GetObjectID())->SetSelectMode(false);
	}

	InstancedSubObjectIndexSelected = -1;
	Container = nullptr;
	bDirtyFlag = true;
	if (OnUpdateFunc != nullptr)
		OnUpdateFunc();
}

glm::dvec3 FEEditorSelectedObject::MouseRay(const double MouseX, const double MouseY) const
{
	glm::dvec2 NormalizedMouseCoords;
	NormalizedMouseCoords.x = (2.0f * MouseX) / ENGINE.GetRenderTargetWidth() - 1;
	NormalizedMouseCoords.y = 1.0f - (2.0f * (MouseY)) / ENGINE.GetRenderTargetHeight();

	const glm::dvec4 ClipCoords = glm::dvec4(NormalizedMouseCoords.x, NormalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 EyeCoords = glm::inverse(ENGINE.GetCamera()->GetProjectionMatrix()) * ClipCoords;
	EyeCoords.z = -1.0f;
	EyeCoords.w = 0.0f;
	glm::dvec3 WorldRay = glm::inverse(ENGINE.GetCamera()->GetViewMatrix()) * EyeCoords;
	WorldRay = glm::normalize(WorldRay);

	return WorldRay;
}

void FEEditorSelectedObject::DetermineEntityUnderMouse(const double MouseX, const double MouseY)
{
	SELECTED.ObjectsUnderMouse.clear();
	SELECTED.InstancedSubObjectsInfo.clear();

	const glm::vec3 MouseRayVector = MouseRay(MouseX, MouseY);
	const std::vector<std::string> EntityList = SCENE.GetEntityList();
	for (size_t i = 0; i < EntityList.size(); i++)
	{
		float dis = 0;
		FEAABB box = SCENE.GetEntity(EntityList[i])->GetAABB();
		if (box.RayIntersect(ENGINE.GetCamera()->GetPosition(), MouseRayVector, dis))
		{
			if (SCENE.GetEntity(EntityList[i])->GetType() == FE_ENTITY_INSTANCED)
			{
				FEEntityInstanced* InstancedEntity = reinterpret_cast<FEEntityInstanced*>(SCENE.GetEntity(EntityList[i]));
				if (InstancedEntity->IsSelectMode())
				{
					InstancedSubObjectsInfo[InstancedEntity] = std::vector<int>();
					for (size_t j = 0; j < InstancedEntity->InstancedAABB.size(); j++)
					{
						if(InstancedEntity->InstancedAABB[j].RayIntersect(ENGINE.GetCamera()->GetPosition(), MouseRayVector, dis))
						{
							InstancedSubObjectsInfo[InstancedEntity].push_back(static_cast<int>(j));
						}
					}
				}

				SELECTED.ObjectsUnderMouse.push_back(InstancedEntity);
			}
			else
			{
				SELECTED.ObjectsUnderMouse.push_back(SCENE.GetEntity(EntityList[i]));
			}
		}
	}

	const std::vector<std::string> TerrainList = SCENE.GetTerrainList();
	for (size_t i = 0; i < TerrainList.size(); i++)
	{
		float dis = 0;
		FEAABB box = SCENE.GetTerrain(TerrainList[i])->GetAABB();
		if (box.RayIntersect(ENGINE.GetCamera()->GetPosition(), MouseRayVector, dis))
		{
			SELECTED.ObjectsUnderMouse.push_back(SCENE.GetTerrain(TerrainList[i]));
		}
	}
}

int FEEditorSelectedObject::GetIndexOfObjectUnderMouse(const double MouseX, const double MouseY)
{
#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!CheckForSelectionisNeeded)
		return -1;
#endif

	CheckForSelectionisNeeded = false;

	PixelAccurateSelectionFB->Bind();
	FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	for (size_t i = 0; i < SELECTED.ObjectsUnderMouse.size(); i++)
	{
#ifdef EDITOR_SELECTION_DEBUG_MODE
		int r = (i + 1) * 50 & 255;
		int g = ((i + 1) * 50 >> 8) & 255;
		int b = ((i + 1) * 50 >> 16) & 255;
#else
		int r = (i + 1) & 255;
		int g = ((i + 1) >> 8) & 255;
		int b = ((i + 1) >> 16) & 255;
#endif
		
		if (SELECTED.ObjectsUnderMouse[i]->GetType() == FE_ENTITY)
		{
			PotentiallySelectedEntity = SCENE.GetEntity(SELECTED.ObjectsUnderMouse[i]->GetObjectID());
			if (!PotentiallySelectedEntity->IsVisible())
				continue;

			for (int j = 0; j < PotentiallySelectedEntity->Prefab->ComponentsCount(); j++)
			{
				FEMaterial* RegularMaterial = PotentiallySelectedEntity->Prefab->GetComponent(j)->GameModel->Material;
				PotentiallySelectedEntity->Prefab->GetComponent(j)->GameModel->Material = PixelAccurateSelectionMaterial;
				PixelAccurateSelectionMaterial->SetBaseColor(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
				PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
				PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
				PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

				RENDERER.RenderEntity(PotentiallySelectedEntity, ENGINE.GetCamera(), false, j);

				PotentiallySelectedEntity->Prefab->GetComponent(j)->GameModel->Material = RegularMaterial;
			}
			PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr);
			PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr, 1);
		}
		else if (SELECTED.ObjectsUnderMouse[i]->GetType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* PotentiallySelectedEntityInstanced = reinterpret_cast<FEEntityInstanced*>(SCENE.GetEntity((SELECTED.ObjectsUnderMouse[i]->GetObjectID())));
			if (!PotentiallySelectedEntityInstanced->IsVisible())
				continue;

			if (InstancedSubObjectsInfo.find(PotentiallySelectedEntityInstanced) == InstancedSubObjectsInfo.end())
			{
				for (int j = 0; j < PotentiallySelectedEntityInstanced->Prefab->ComponentsCount(); j++)
				{
					FEMaterial* RegularMaterial = PotentiallySelectedEntityInstanced->Prefab->GetComponent(j)->GameModel->Material;
					PotentiallySelectedEntityInstanced->Prefab->GetComponent(j)->GameModel->Material = PixelAccurateSelectionMaterial;

					PixelAccurateSelectionMaterial->SetBaseColor(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
					PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
					PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
					PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

					PixelAccurateSelectionMaterial->Shader = FEPixelAccurateInstancedSelection;
					FEMaterial* RegularBillboardMaterials = PotentiallySelectedEntityInstanced->Prefab->GetComponent(j)->GameModel->GetBillboardMaterial();
					PotentiallySelectedEntityInstanced->Prefab->GetComponent(j)->GameModel->SetBillboardMaterial(PixelAccurateSelectionMaterial);

					RENDERER.RenderEntityInstanced(PotentiallySelectedEntityInstanced, ENGINE.GetCamera(), nullptr, false, false, j);

					PixelAccurateSelectionMaterial->Shader = FEPixelAccurateSelection;
					PotentiallySelectedEntityInstanced->Prefab->GetComponent(j)->GameModel->SetBillboardMaterial(RegularBillboardMaterials);
					PotentiallySelectedEntityInstanced->Prefab->GetComponent(j)->GameModel->Material = RegularMaterial;

					PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr);
					PixelAccurateSelectionMaterial->SetAlbedoMap(nullptr, 1);
				}
			}
		}
		else if (SELECTED.ObjectsUnderMouse[i]->GetType() == FE_TERRAIN)
		{
			FETerrain* PotentiallySelectedTerrain = SCENE.GetTerrain(SELECTED.ObjectsUnderMouse[i]->GetObjectID());
			if (PotentiallySelectedTerrain != nullptr)
			{
				if (!PotentiallySelectedTerrain->IsVisible())
					continue;

				PotentiallySelectedTerrain->Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
				PotentiallySelectedTerrain->Shader->GetParameter("baseColor")->UpdateData(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
				RENDERER.RenderTerrain(PotentiallySelectedTerrain, ENGINE.GetCamera());
				PotentiallySelectedTerrain->Shader->GetParameter("baseColor")->UpdateData(glm::vec3(1.0f));
				PotentiallySelectedTerrain->Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
			}
		}
	}

	int LastColorShiftIndex = static_cast<int>(SELECTED.ObjectsUnderMouse.size() - 1);

	auto it = InstancedSubObjectsInfo.begin();
	while (it != InstancedSubObjectsInfo.end())
	{
		for (size_t j = 0; j < it->second.size(); j++)
		{
			LastColorShiftIndex++;
			int r = (LastColorShiftIndex + 1) & 255;
			int g = ((LastColorShiftIndex + 1) >> 8) & 255;
			int b = ((LastColorShiftIndex + 1) >> 16) & 255;

			static FEEntity* DummyEntity = new FEEntity(it->first->Prefab, "dummyEntity");
			DummyEntity->Prefab = it->first->Prefab;
			DummyEntity->Transform = FETransformComponent(it->first->GetTransformedInstancedMatrix(it->second[j]));

			for (int k = 0; k < DummyEntity->Prefab->ComponentsCount(); k++)
			{
				FEMaterial* RegularMaterial = it->first->Prefab->GetComponent(k)->GameModel->Material;
				DummyEntity->Prefab->GetComponent(k)->GameModel->Material = PixelAccurateSelectionMaterial;
				PixelAccurateSelectionMaterial->SetBaseColor(glm::vec3(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f));
				PixelAccurateSelectionMaterial->ClearAllTexturesInfo();
				PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
				PixelAccurateSelectionMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

				RENDERER.RenderEntity(DummyEntity, ENGINE.GetCamera(), false, k);
			
				it->first->Prefab->GetComponent(k)->GameModel->Material = RegularMaterial;
			}
		}
		it++;
	}

	FE_GL_ERROR(glReadPixels(static_cast<GLint>(MouseX), GLint(ENGINE.GetRenderTargetHeight() - MouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, ColorUnderMouse));
	PixelAccurateSelectionFB->UnBind();
	FE_GL_ERROR(glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w));

#ifndef EDITOR_SELECTION_DEBUG_MODE
	if (!SELECTED.ObjectsUnderMouse.empty())
	{
		ColorIndex = 0;
		ColorIndex |= static_cast<int>(ColorUnderMouse[2]);
		ColorIndex <<= 8;
		ColorIndex |= static_cast<int>(ColorUnderMouse[1]);
		ColorIndex <<= 8;
		ColorIndex |= static_cast<int>(ColorUnderMouse[0]);

		ColorIndex -= 1;

		if (ColorIndex != -1 && ColorIndex >= static_cast<int>(SELECTED.ObjectsUnderMouse.size()))
		{
			ColorIndex -= static_cast<int>(SELECTED.ObjectsUnderMouse.size());

			const FEEntityInstanced* SelectedSubObjectInInstance = nullptr;
			auto it = InstancedSubObjectsInfo.begin();
			while (it != InstancedSubObjectsInfo.end())
			{
				if (ColorIndex < static_cast<int>(it->second.size()))
				{
					InstancedSubObjectIndexSelected = it->second[ColorIndex];
					SelectedSubObjectInInstance = it->first;
					break;
				}

				ColorIndex -= static_cast<int>(it->second.size());
				it++;
			}

			if (SelectedSubObjectInInstance != nullptr)
			{
				for (size_t i = 0; i < SELECTED.ObjectsUnderMouse.size(); i++)
				{
					if (SELECTED.ObjectsUnderMouse[i]->GetObjectID() == SelectedSubObjectInInstance->GetObjectID())
					{
						return static_cast<int>(i);
					}
				}
			}
		}
		else if (ColorIndex != -1)
		{
			return ColorIndex;
		}
	}

	SELECTED.Clear();
	return -1;
#else
	ColorIndex = 0;
	ColorIndex |= int(ColorUnderMouse[2]);
	ColorIndex <<= 8;
	ColorIndex |= int(ColorUnderMouse[1]);
	ColorIndex <<= 8;
	ColorIndex |= int(ColorUnderMouse[0]);

	ColorIndex /= 50;
	ColorIndex -= 1;

	return -1;
#endif
}

void FEEditorSelectedObject::OnCameraUpdate() const
{
	HALO_SELECTION_EFFECT.HaloObjectsFb->Bind();
	HALO_SELECTION_EFFECT.HaloMaterial->ClearAllTexturesInfo();
	HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(glm::vec3(1.0f, 0.25f, 0.0f));
	FE_GL_ERROR(glViewport(0, 0, RENDERER.SceneToTextureFB->GetWidth(), RENDERER.SceneToTextureFB->GetHeight()));
	FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));

	if (Container == nullptr)
	{
		HALO_SELECTION_EFFECT.HaloObjectsFb->UnBind();
		FE_GL_ERROR(glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w));
		HALO_SELECTION_EFFECT.PostProcess->bActive = true;
		return;
	}
	
	if (Container->GetType() == FE_ENTITY)
	{
		FEEntity* SelectedEntity = SCENE.GetEntity(Container->GetObjectID());

		for (int i = 0; i < SelectedEntity->Prefab->ComponentsCount(); i++)
		{
			FEMaterial* RegularMaterial = SelectedEntity->Prefab->GetComponent(i)->GameModel->Material;

			SelectedEntity->Prefab->GetComponent(i)->GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;
			HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
			HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);
			
			RENDERER.RenderEntity(SelectedEntity, ENGINE.GetCamera(), false, i);
			
			SelectedEntity->Prefab->GetComponent(i)->GameModel->Material = RegularMaterial;
		}
	}
	else if (Container->GetType() == FE_ENTITY_INSTANCED)
	{
		FEEntityInstanced* SelectedEntity = reinterpret_cast<FEEntityInstanced*>(SCENE.GetEntity(Container->GetObjectID()));

		if (InstancedSubObjectIndexSelected != -1)
		{
			static FEEntity* DummyEntity = new FEEntity(SelectedEntity->Prefab, "dummyEntity");
			DummyEntity->Prefab = SelectedEntity->Prefab;
			DummyEntity->Transform = FETransformComponent(SelectedEntity->GetTransformedInstancedMatrix(InstancedSubObjectIndexSelected));

			for (int i = 0; i < DummyEntity->Prefab->ComponentsCount(); i++)
			{
				FEMaterial* RegularMaterial = DummyEntity->Prefab->GetComponent(i)->GameModel->Material;

				DummyEntity->Prefab->GetComponent(i)->GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;

				HALO_SELECTION_EFFECT.HaloMaterial->SetBaseColor(glm::vec3(0.61f, 0.86f, 1.0f));
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

				RENDERER.RenderEntity(DummyEntity, ENGINE.GetCamera(), false, i);

				SelectedEntity->Prefab->GetComponent(i)->GameModel->Material = RegularMaterial;
			}
		}
		else
		{
			for (int i = 0; i < SelectedEntity->Prefab->ComponentsCount(); i++)
			{
				FEMaterial* RegularMaterial = SelectedEntity->Prefab->GetComponent(i)->GameModel->Material;
				SelectedEntity->Prefab->GetComponent(i)->GameModel->Material = HALO_SELECTION_EFFECT.HaloMaterial;
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap());
				HALO_SELECTION_EFFECT.HaloMaterial->SetAlbedoMap(RegularMaterial->GetAlbedoMap(1), 1);

				HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawInstancedObjectShader;
				FEMaterial* RegularBillboardMaterial = SelectedEntity->Prefab->GetComponent(i)->GameModel->GetBillboardMaterial();
				SelectedEntity->Prefab->GetComponent(i)->GameModel->SetBillboardMaterial(HALO_SELECTION_EFFECT.HaloMaterial);

				RENDERER.RenderEntityInstanced(SelectedEntity, ENGINE.GetCamera(), nullptr, false, false, i);

				HALO_SELECTION_EFFECT.HaloMaterial->Shader = HALO_SELECTION_EFFECT.HaloDrawObjectShader;
				SelectedEntity->Prefab->GetComponent(i)->GameModel->SetBillboardMaterial(RegularBillboardMaterial);
				SelectedEntity->Prefab->GetComponent(i)->GameModel->Material = RegularMaterial;
			}
		}
	}
	else if (Container->GetType() == FE_TERRAIN)
	{
		FETerrain* SelectedTerrain = SCENE.GetTerrain(Container->GetObjectID());

		SelectedTerrain->Shader = RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
		SelectedTerrain->Shader->GetParameter("baseColor")->UpdateData(glm::vec3(1.0f, 0.25f, 0.0f));
		const float RegularLODLevel = SelectedTerrain->GetLODLevel();
		SelectedTerrain->SetLODLevel(0.0f);
		RENDERER.RenderTerrain(SelectedTerrain, ENGINE.GetCamera());
		SelectedTerrain->SetLODLevel(RegularLODLevel);
		SelectedTerrain->Shader->GetParameter("baseColor")->UpdateData(glm::vec3(1.0f));
		SelectedTerrain->Shader = RESOURCE_MANAGER.GetShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
	}

	HALO_SELECTION_EFFECT.HaloObjectsFb->UnBind();
	FE_GL_ERROR(glClearColor(FE_CLEAR_COLOR.x, FE_CLEAR_COLOR.y, FE_CLEAR_COLOR.z, FE_CLEAR_COLOR.w));
	HALO_SELECTION_EFFECT.PostProcess->bActive = true;
}

int FEEditorSelectedObject::DebugGetLastColorIndex() const
{
	return ColorIndex;
}

void FEEditorSelectedObject::SetSelectedByIndex(const size_t Index)
{
	if (Index < 0 || Index >= ObjectsUnderMouse.size())
		return;

	if (Container != nullptr)
	{
		if (SCENE.GetEntityInstanced(Container->GetObjectID()) != nullptr)
		{
			if (ObjectsUnderMouse[Index]->GetObjectID() != Container->GetObjectID())
			{
				SCENE.GetEntityInstanced(Container->GetObjectID())->SetSelectMode(false);
				InstancedSubObjectIndexSelected = -1;
			}
		}
	}

	Container = ObjectsUnderMouse[Index];
	if (OnUpdateFunc != nullptr)
		OnUpdateFunc();
}

FEEntity* FEEditorSelectedObject::GetEntity() const
{
	if (Container == nullptr)
		return nullptr;


	if (Container->GetType() == FE_ENTITY || Container->GetType() == FE_ENTITY_INSTANCED)
	{
		return SCENE.GetEntity(Container->GetObjectID());
	}

	return nullptr;
}

FELight* FEEditorSelectedObject::GetLight() const
{
	if (Container == nullptr)
		return nullptr;

	if (Container->GetType() == FE_DIRECTIONAL_LIGHT || Container->GetType() == FE_SPOT_LIGHT || Container->GetType() == FE_POINT_LIGHT)
	{
		return SCENE.GetLight(Container->GetObjectID());
	}

	return nullptr;
}

FETerrain* FEEditorSelectedObject::GetTerrain() const
{
	if (Container == nullptr)
		return nullptr;

	if (Container->GetType() == FE_TERRAIN)
	{
		return SCENE.GetTerrain(Container->GetObjectID());
	}

	return nullptr;
}

//template <class T>
//T* FEEditorSelectedObject::getSelected(FEObjectType type)
//{
//	if (container == nullptr)
//		return nullptr;
//
//	if (type == FE_ENTITY || type == FE_ENTITY_INSTANCED)
//	{
//		return SCENE.getEntity(container->getObjectID());
//	}
//
//	return nullptr;
//}