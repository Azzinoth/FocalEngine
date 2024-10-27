#include "../../FENativeScriptConnector.h"
using namespace FocalEngine;

// DO NOT CHANGE THIS LINE.
SET_MODULE_ID("2B7956623302254F620A675F");

class FreeCameraController : public FENativeScriptCore
{
	int CorrectionToSensitivity = 2;

	int LastMouseX = 0;
	int LastMouseY = 0;

	int RenderTargetCenterX = 0;
	int RenderTargetCenterY = 0;

	float CurrentMouseXAngle = 0.0f;
	float CurrentMouseYAngle = 0.0f;

	bool bLastFrameWasActive = false;
	bool bFirstFrameActive = true;

	void SetCursorToCenter();
	void UpdateViewMatrix();
public:
	void Awake() override;
	void OnUpdate(double DeltaTime) override;
	void OnDestroy() override { /* ... */ }

	float MovementSpeed = 10.0f;
};

REGISTER_SCRIPT(FreeCameraController)
RUN_IN_EDITOR_MODE(FreeCameraController)
REGISTER_SCRIPT_FIELD(FreeCameraController, float, MovementSpeed)

class ModelViewCameraController : public FENativeScriptCore
{
	int LastMouseX = 0;
	int LastMouseY = 0;

	double CurrentPolarAngle = 90.0;
	double CurrentAzimutAngle = 90.0;

	glm::dvec3 PolarToCartesian(double PolarAngle, double AzimutAngle, const double R);
	void UpdateViewMatrix();
public:
	void Awake() override;
	void OnUpdate(double DeltaTime) override;
	void OnDestroy() override { /* ... */ }

	float DistanceToModel = 10.0;
	glm::vec3 TargetPosition = glm::vec3(0.0f);
	float MouseWheelSensitivity = 1.0f;
};

REGISTER_SCRIPT(ModelViewCameraController)
RUN_IN_EDITOR_MODE(ModelViewCameraController)
REGISTER_SCRIPT_FIELD(ModelViewCameraController, float, DistanceToModel)
REGISTER_SCRIPT_FIELD(ModelViewCameraController, glm::vec3, TargetPosition)
REGISTER_SCRIPT_FIELD(ModelViewCameraController, float, MouseWheelSensitivity)