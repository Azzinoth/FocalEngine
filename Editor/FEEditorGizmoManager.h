#pragma once

#include "../FEngine.h"
#include "../Editor/FEEditorSelectedObject.h"
using namespace FocalEngine;

class FEEditor;
struct SelectedObject;

const int TRANSFORM_GIZMOS = 0;
const int SCALE_GIZMOS = 1;
const int ROTATE_GIZMOS = 2;

class GizmoManager
{
	friend FEEditor;
	friend SelectedObject;
public:
	SINGLETON_PUBLIC_PART(GizmoManager)

	float GizmosScale = 0.00175f;
	int GizmosState = TRANSFORM_GIZMOS;

	FEEntity* TransformationXGizmoEntity = nullptr;
	FEEntity* TransformationYGizmoEntity = nullptr;
	FEEntity* TransformationZGizmoEntity = nullptr;

	bool bTransformationXGizmoActive = false;
	bool bTransformationYGizmoActive = false;
	bool bTransformationZGizmoActive = false;

	FEEntity* TransformationXyGizmoEntity = nullptr;
	FEEntity* TransformationYzGizmoEntity = nullptr;
	FEEntity* TransformationXzGizmoEntity = nullptr;

	bool bTransformationXYGizmoActive = false;
	bool bTransformationYZGizmoActive = false;
	bool bTransformationXZGizmoActive = false;

	// scale part
	FEEntity* ScaleXGizmoEntity = nullptr;
	FEEntity* ScaleYGizmoEntity = nullptr;
	FEEntity* ScaleZGizmoEntity = nullptr;

	bool bScaleXGizmoActive = false;
	bool bScaleYGizmoActive = false;
	bool bScaleZGizmoActive = false;

	// rotate part
	FEEntity* RotateXGizmoEntity = nullptr;
	FEEntity* RotateYGizmoEntity = nullptr;
	FEEntity* RotateZGizmoEntity = nullptr;

	bool bRotateXGizmoActive = false;
	bool bRotateYGizmoActive = false;
	bool bRotateZGizmoActive = false;

	glm::vec3 RotateXStandardRotation = glm::vec3(0.0f, 0.0f, -90.0f);
	glm::vec3 RotateYStandardRotation = glm::vec3(0.0f);
	glm::vec3 RotateZStandardRotation = glm::vec3(90.0f, 0.0f, 90.0f);

	FETexture* TransformationGizmoIcon = nullptr;
	FETexture* ScaleGizmoIcon = nullptr;
	FETexture* RotateGizmoIcon = nullptr;

	void InitializeResources();
	void ReInitializeEntities();

	void DeactivateAllGizmo();
	void HideAllGizmo();
	void UpdateGizmoState(int NewState);
	void Render();
	bool WasSelected(int Index);

	void MouseMove(double LastMouseX, double LastMouseY, double MouseX, double MouseY);

private:
	SINGLETON_PRIVATE_PART(GizmoManager)

	double LastMouseX = 0, LastMouseY = 0, MouseX = 0, MouseY = 0;

	glm::vec3 GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal);
	glm::vec3 GetMousePositionDifferenceOnPlane(glm::vec3 PlaneNormal, glm::vec3& LastMousePointOnPlane);
	bool RaysIntersection(const glm::vec3& FRayOrigin, const glm::vec3& FRayDirection,
	                      const glm::vec3& SRayOrigin, const glm::vec3& SRayDirection,
	                      float& Ft, float& St) const;

	void MouseMoveTransformationGizmos();
	void MouseMoveScaleGizmos();
	void MouseMoveRotateGizmos();

	static void OnSelectedObjectUpdate();

	FETransformComponent GetTransformComponentOfSelectedObject();
	void ApplyChangesToSelectedObject(FETransformComponent Changes);
};

#define GIZMO_MANAGER GizmoManager::getInstance()