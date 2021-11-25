#pragma once

#include "../FEngine.h"
#include "../Editor/FEEditorSelectedObject.h"
using namespace FocalEngine;

class FEEditor;
struct selectedObject;

namespace FEGizmoManager
{
	const int TRANSFORM_GIZMOS = 0;
	const int SCALE_GIZMOS = 1;
	const int ROTATE_GIZMOS = 2;

	class GizmoManager
	{
		friend FEEditor;
		friend selectedObject;
	public:
		SINGLETON_PUBLIC_PART(GizmoManager)

		float gizmosScale = 0.00175f;
		int gizmosState = TRANSFORM_GIZMOS;

		FEEntity* transformationXGizmoEntity = nullptr;
		FEEntity* transformationYGizmoEntity = nullptr;
		FEEntity* transformationZGizmoEntity = nullptr;

		bool transformationXGizmoActive = false;
		bool transformationYGizmoActive = false;
		bool transformationZGizmoActive = false;

		FEEntity* transformationXYGizmoEntity = nullptr;
		FEEntity* transformationYZGizmoEntity = nullptr;
		FEEntity* transformationXZGizmoEntity = nullptr;

		bool transformationXYGizmoActive = false;
		bool transformationYZGizmoActive = false;
		bool transformationXZGizmoActive = false;

		// scale part
		FEEntity* scaleXGizmoEntity = nullptr;
		FEEntity* scaleYGizmoEntity = nullptr;
		FEEntity* scaleZGizmoEntity = nullptr;

		bool scaleXGizmoActive = false;
		bool scaleYGizmoActive = false;
		bool scaleZGizmoActive = false;

		// rotate part
		FEEntity* rotateXGizmoEntity = nullptr;
		FEEntity* rotateYGizmoEntity = nullptr;
		FEEntity* rotateZGizmoEntity = nullptr;

		bool rotateXGizmoActive = false;
		bool rotateYGizmoActive = false;
		bool rotateZGizmoActive = false;

		glm::vec3 rotateXStandardRotation = glm::vec3(0.0f, 0.0f, -90.0f);
		glm::vec3 rotateYStandardRotation = glm::vec3(0.0f);
		glm::vec3 rotateZStandardRotation = glm::vec3(90.0f, 0.0f, 90.0f);

		FETexture* transformationGizmoIcon = nullptr;
		FETexture* scaleGizmoIcon = nullptr;
		FETexture* rotateGizmoIcon = nullptr;

		void initializeResources();
		void reInitializeEntities();

		void deactivateAllGizmo();
		void hideAllGizmo();
		void updateGizmoState(int newState);
		void render();
		bool wasSelected(int index);

		void mouseMove(double lastMouseX, double lastMouseY, double mouseX, double mouseY);

	private:
		SINGLETON_PRIVATE_PART(GizmoManager)

		double lastMouseX = 0, lastMouseY = 0, mouseX = 0, mouseY = 0;

		glm::vec3 getMousePositionDifferenceOnPlane(glm::vec3 planeNormal);
		glm::vec3 getMousePositionDifferenceOnPlane(glm::vec3 planeNormal, glm::vec3& lastMousePointOnPlane);
		bool raysIntersection(glm::vec3& fRayOrigin, glm::vec3& fRayDirection,
							  glm::vec3& sRayOrigin, glm::vec3& sRayDirection,
							  float& fT, float& sT);

		void mouseMoveTransformationGizmos();
		void mouseMoveScaleGizmos();
		void mouseMoveRotateGizmos();

		static void onSelectedObjectUpdate();

		FETransformComponent getTransformComponentOfSelectedObject();
		void applyChangesToSelectedObject(FETransformComponent changes);
	};
}

#define GIZMO_MANAGER FEGizmoManager::GizmoManager::getInstance()
