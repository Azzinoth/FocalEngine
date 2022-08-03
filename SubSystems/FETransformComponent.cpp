#include "FETransformComponent.h"
using namespace FocalEngine;

FETransformComponent::FETransformComponent()
{
	Position = glm::vec3(0.0f);
	RotationAngles = glm::vec3(0.0f);
	RotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
	Scale = glm::vec3(1.0f, 1.0f, 1.0f);

	Update();
}

FETransformComponent::FETransformComponent(glm::mat4 Matrix)
{
	Position = glm::vec3(Matrix[3][0], Matrix[3][1], Matrix[3][2]);
	Scale = glm::vec3(glm::length(Matrix[0]), glm::length(Matrix[1]), glm::length(Matrix[2]));

	Matrix[3][0] = 0.0f;
	Matrix[3][1] = 0.0f;
	Matrix[3][2] = 0.0f;

	Matrix[0] /= Scale.x;
	Matrix[1] /= Scale.y;
	Matrix[2] /= Scale.z;

	RotationQuaternion = glm::quat_cast(Matrix);
	RotationAngles = glm::eulerAngles(RotationQuaternion) * 180.0f / glm::pi<float>();

	Update();
}

FETransformComponent::~FETransformComponent()
{
}

glm::vec3 FETransformComponent::GetPosition() const
{
	return Position;
}

glm::vec3 FETransformComponent::GetRotation() const
{
	return RotationAngles;
}

glm::vec3 FETransformComponent::GetScale() const
{
	return Scale;
}

void FETransformComponent::SetPosition(const glm::vec3 NewPosition)
{
	Position = NewPosition;
	Update();
}

void FETransformComponent::RotateQuaternion(const float Angle, const glm::vec3 Axis)
{
	RotationQuaternion = glm::quat(cos(Angle / 2),
								   Axis.x * sin(Angle / 2),
								   Axis.y * sin(Angle / 2),
								   Axis.z * sin(Angle / 2)) * RotationQuaternion;
}

void FETransformComponent::SetRotation(const glm::vec3 NewRotation)
{
	if (RotationAngles == NewRotation)
		return;

	RotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
	RotateQuaternion(static_cast<float>(NewRotation.x) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	RotateQuaternion(static_cast<float>(NewRotation.y) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	RotateQuaternion(static_cast<float>(NewRotation.z) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));
	
	RotationAngles = NewRotation;
	Update();
}

void FETransformComponent::RotateByQuaternion(const glm::quat Quaternion)
{
	RotationQuaternion = Quaternion * RotationQuaternion;
	glm::vec3 NewRotationAngle = glm::eulerAngles(GetQuaternion());

	NewRotationAngle.x /= ANGLE_TORADIANS_COF;
	NewRotationAngle.y /= ANGLE_TORADIANS_COF;
	NewRotationAngle.z /= ANGLE_TORADIANS_COF;

	RotationAngles = NewRotationAngle;
	Update();
}

void FETransformComponent::ChangeScaleUniformlyBy(const float Delta)
{
	Scale += Delta;
	Update();
}

void FETransformComponent::ChangeXScaleBy(const float Delta)
{
	if (bUniformScaling)
	{
		ChangeScaleUniformlyBy(Delta);
	}
	else
	{
		Scale[0] += Delta;
	}

	Update();
}

void FETransformComponent::ChangeYScaleBy(const float Delta)
{
	if (bUniformScaling)
	{
		ChangeScaleUniformlyBy(Delta);
	}
	else
	{
		Scale[1] += Delta;
	}

	Update();
}

void FETransformComponent::ChangeZScaleBy(const float Delta)
{
	if (bUniformScaling)
	{
		ChangeScaleUniformlyBy(Delta);
	}
	else
	{
		Scale[2] += Delta;
	}

	Update();
}

void FETransformComponent::SetScale(const glm::vec3 NewScale)
{
	Scale = NewScale;
	Update();
}

void FETransformComponent::Update()
{
	TransformMatrix = glm::mat4(1.0);
	TransformMatrix = glm::translate(TransformMatrix, Position);

	TransformMatrix *= glm::toMat4(RotationQuaternion);
	TransformMatrix = glm::scale(TransformMatrix, glm::vec3(Scale[0], Scale[1], Scale[2]));

	if (PreviousTransformMatrix != TransformMatrix)
		bDirtyFlag = true;
	PreviousTransformMatrix = TransformMatrix;
}

glm::mat4 FETransformComponent::GetTransformMatrix() const
{
	return TransformMatrix;
}

glm::quat FETransformComponent::GetQuaternion() const
{
	return RotationQuaternion;
}

bool FETransformComponent::IsDirty() const
{
	return bDirtyFlag;
}

void FETransformComponent::SetDirtyFlag(const bool NewValue)
{
	bDirtyFlag = NewValue;
}

void FETransformComponent::ForceSetTransformMatrix(const glm::mat4 NewValue)
{
	TransformMatrix = NewValue;
}

FETransformComponent FETransformComponent::Combine(const FETransformComponent& Other) const
{
	FETransformComponent result;

	result.SetPosition(GetPosition() + Other.GetPosition());
	result.SetRotation(GetRotation() + Other.GetRotation());
	result.SetScale(GetScale() * Other.GetScale());

	return result;
}

bool FETransformComponent::IsUniformScalingSet() const
{
	return bUniformScaling;
}

void FETransformComponent::SetUniformScaling(const bool NewValue)
{
	bUniformScaling = NewValue;
}