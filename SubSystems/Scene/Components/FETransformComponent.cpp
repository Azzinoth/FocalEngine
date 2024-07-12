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

glm::vec3 FETransformComponent::GetPosition(bool bLocalSpace) const
{
	if (!bLocalSpace)
	{
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotationQuaternion;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(WorldSpaceMatrix, DoubleTranslation, DoubleRotationQuaternion, DoubleScale))
		{
			return DoubleTranslation;
		}
	}

	return Position;
}

glm::vec3 FETransformComponent::GetRotation(bool bLocalSpace) const
{
	if (!bLocalSpace)
	{
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotationQuaternion;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(WorldSpaceMatrix, DoubleTranslation, DoubleRotationQuaternion, DoubleScale))
		{
			glm::vec3 RotationAnglesInRadians = glm::eulerAngles(DoubleRotationQuaternion);
			glm::vec3 RotationAnglesInDegrees = RotationAnglesInRadians * RADIANS_TOANGLE_COF;

			return RotationAnglesInDegrees;
		}
	}

	return RotationAngles;
}

glm::quat FETransformComponent::GetQuaternion(bool bLocalSpace) const
{
	if (!bLocalSpace)
	{
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotationQuaternion;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(WorldSpaceMatrix, DoubleTranslation, DoubleRotationQuaternion, DoubleScale))
		{
			return DoubleRotationQuaternion;
		}
	}

	return RotationQuaternion;
}

glm::vec3 FETransformComponent::GetScale(bool bLocalSpace) const
{
	if (!bLocalSpace)
	{
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotationQuaternion;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(WorldSpaceMatrix, DoubleTranslation, DoubleRotationQuaternion, DoubleScale))
		{
			return DoubleScale;
		}
	}

	return Scale;
}

glm::mat4 FETransformComponent::GetParentMatrix() const
{
	if (bSceneIndependent)
		return glm::identity<glm::mat4>();

	// We assume that WorldSpaceMatrix was set in the previous frame like this:
	// WorldSpaceMatrix = ParentWorldSpace * LocalSpaceMatrix;
	// Therefore, we can use it to extract ParentWorldSpace.
	glm::mat4 ParentWorldSpace = WorldSpaceMatrix * glm::inverse(LocalSpaceMatrix);
	return ParentWorldSpace;
}

void FETransformComponent::MoveAlongAxis(const glm::vec3& Axis, float MovementValue, bool bLocalSpace)
{
	glm::vec3 NewPosition = GetPosition();
	glm::vec3 LocalAlternativeAxis = Axis * MovementValue;

	if (!bLocalSpace)
		LocalAlternativeAxis = glm::inverse(GetParentMatrix()) * glm::vec4(LocalAlternativeAxis, 0.0f);

	NewPosition += LocalAlternativeAxis;
	SetPosition(NewPosition);
}

void FETransformComponent::SetPosition(const glm::vec3 NewPosition, bool bLocalSpace)
{
	if (bLocalSpace)
	{
		Position = NewPosition;
		Update();
	}

	if (!bLocalSpace)
	{
		glm::vec3 CurrentWorldPosition = GetPosition(bLocalSpace);
		glm::vec3 Difference = NewPosition - CurrentWorldPosition;

		MoveAlongAxis(glm::vec3(1, 0, 0), Difference.x, bLocalSpace);
		MoveAlongAxis(glm::vec3(0, 1, 0), Difference.y, bLocalSpace);
		MoveAlongAxis(glm::vec3(0, 0, 1), Difference.z, bLocalSpace);
	}

	SetDirtyFlag(true);
}

void FETransformComponent::RotateQuaternion(const float Angle, const glm::vec3 Axis)
{
	RotationQuaternion = glm::quat(cos(Angle / 2),
								   Axis.x * sin(Angle / 2),
								   Axis.y * sin(Angle / 2),
								   Axis.z * sin(Angle / 2)) * RotationQuaternion;
}

void FETransformComponent::RotateAroundAxis(const glm::vec3& Axis, const float& RotationAmount, bool bLocalSpace)
{
	glm::vec3 FinalAxis = Axis;

	if (!bLocalSpace)
	{
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotation;
		glm::dvec3 DoubleTranslation;
		glm::dvec3 DoubleSkew;
		glm::dvec4 DoublePerspective;
		glm::dmat4 ParentMatrix = GetParentMatrix();
		bool Success = glm::decompose(ParentMatrix, DoubleScale, DoubleRotation, DoubleTranslation, DoubleSkew, DoublePerspective);
		if (Success)
		{
			FinalAxis = glm::inverse(glm::quat(DoubleRotation)) * glm::vec3(Axis);
		}
	}

	glm::quat LocalRotationQuaternion = glm::quat(cos(RotationAmount * ANGLE_TORADIANS_COF / 2),
												  FinalAxis.x * sin(RotationAmount * ANGLE_TORADIANS_COF / 2),
												  FinalAxis.y * sin(RotationAmount * ANGLE_TORADIANS_COF / 2),
												  FinalAxis.z * sin(RotationAmount * ANGLE_TORADIANS_COF / 2));

	RotationQuaternion = LocalRotationQuaternion * RotationQuaternion;
	Update();
}

void FETransformComponent::SetRotation(const glm::vec3 NewRotation, bool bLocalSpace)
{
	if (RotationAngles == NewRotation)
		return;

	glm::vec3 CurrentWorldRotation = GetRotation(false);
	if (bLocalSpace)
	{
		RotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
		RotateQuaternion(static_cast<float>(NewRotation.x) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
		RotateQuaternion(static_cast<float>(NewRotation.y) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
		RotateQuaternion(static_cast<float>(NewRotation.z) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

		RotationAngles = NewRotation;
		Update();
	}
	
	if (!bLocalSpace)
	{
		glm::vec3 Difference = NewRotation - CurrentWorldRotation;
		RotateAroundAxis(glm::vec3(1, 0, 0), Difference.x, bLocalSpace);
		RotateAroundAxis(glm::vec3(0, 1, 0), Difference.y, bLocalSpace);
		RotateAroundAxis(glm::vec3(0, 0, 1), Difference.z, bLocalSpace);
	}
	
	SetDirtyFlag(true);
}

void FETransformComponent::SetQuaternion(glm::quat Quaternion, bool bLocalSpace)
{
	glm::mat4 ParentMatrix;
	if (!bLocalSpace)
	{
		// It is important to get ParentMatrix before Update().
		ParentMatrix = GetParentMatrix();
	}

	RotationQuaternion = Quaternion;
	glm::vec3 NewRotationAngle = glm::eulerAngles(Quaternion);

	NewRotationAngle.x /= ANGLE_TORADIANS_COF;
	NewRotationAngle.y /= ANGLE_TORADIANS_COF;
	NewRotationAngle.z /= ANGLE_TORADIANS_COF;

	RotationAngles = NewRotationAngle;
	Update();

	if (!bLocalSpace)
	{
		// If we did not have scene hierarchy, we can just set LocalSpaceMatrix
		// But since we have it, we need to calculate LocalSpaceMatrix to sustain the new world rotation.
		LocalSpaceMatrix = glm::inverse(ParentMatrix) * LocalSpaceMatrix;

		// Now we will extract new local rotation.
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotationQuaternion;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(LocalSpaceMatrix, DoubleTranslation, DoubleRotationQuaternion, DoubleScale))
		{
			RotationQuaternion = DoubleRotationQuaternion;
			RotationAngles = glm::eulerAngles(RotationQuaternion) * RADIANS_TOANGLE_COF;
		}
	}

	SetDirtyFlag(true);
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

	SetDirtyFlag(true);
}

void FETransformComponent::SetScale(const glm::vec3 NewScale, bool bLocalSpace)
{
	glm::mat4 ParentMatrix;
	if (!bLocalSpace)
	{
		// It is important to get ParentMatrix before Update().
		ParentMatrix = GetParentMatrix();
	}

	Scale = NewScale;
	Update();

	if (!bLocalSpace)
	{
		// If we did not have a scene hierarchy, we could just set LocalSpaceMatrix.
		// But since we have it, we need to calculate LocalSpaceMatrix to sustain the new world scale.
		LocalSpaceMatrix = glm::inverse(ParentMatrix) * LocalSpaceMatrix;

		// Now we will extract new local Scale.
		glm::dvec3 DoubleScale;
		glm::dquat DoubleRotationQuaternion;
		glm::dvec3 DoubleTranslation;
		if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(LocalSpaceMatrix, DoubleTranslation, DoubleRotationQuaternion, DoubleScale))
		{
			Scale = DoubleScale;
		}
	}

	SetDirtyFlag(true);
}

void FETransformComponent::Update()
{
	LocalSpaceMatrix = glm::identity<glm::mat4>();
	LocalSpaceMatrix = glm::translate(LocalSpaceMatrix, Position);
	LocalSpaceMatrix *= glm::toMat4(RotationQuaternion);
	LocalSpaceMatrix = glm::scale(LocalSpaceMatrix, glm::vec3(Scale[0], Scale[1], Scale[2]));
}

glm::mat4 FETransformComponent::GetWorldMatrix() const
{
	if (bSceneIndependent)
		return LocalSpaceMatrix;

	//FIX ME! This is a temporary solution also it is not working as expected with entities that are in origin.
	// Later it should be avoided because of performance issues.
	// 
	// If component is not part of scene hierarchy or it was not updated in the previous frame, we should intervene.
	// To preserve expected behavior.
	if (GEOMETRY.IsEpsilonEqual(WorldSpaceMatrix, glm::identity<glm::mat4>()))
	{
		return WorldSpaceMatrix * LocalSpaceMatrix;
	}

	return WorldSpaceMatrix;
}

void FETransformComponent::ForceSetWorldMatrix(glm::mat4 NewMatrix)
{
	WorldSpaceMatrix = NewMatrix;
}

glm::mat4 FETransformComponent::GetLocalMatrix() const
{
	return LocalSpaceMatrix;
}

void FETransformComponent::ForceSetLocalMatrix(glm::mat4 NewMatrix)
{
	LocalSpaceMatrix = NewMatrix;
}

bool FETransformComponent::IsDirty() const
{
	return bDirtyFlag;
}

void FETransformComponent::SetDirtyFlag(const bool NewValue)
{
	bDirtyFlag = NewValue;
}

FETransformComponent FETransformComponent::Combine(const FETransformComponent& Other) const
{
	FETransformComponent Result;

	Result.SetPosition(GetPosition() + Other.GetPosition());
	Result.SetRotation(GetRotation() + Other.GetRotation());
	Result.SetScale(GetScale() * Other.GetScale());

	return Result;
}

bool FETransformComponent::IsUniformScalingSet() const
{
	return bUniformScaling;
}

void FETransformComponent::SetUniformScaling(const bool NewValue)
{
	bUniformScaling = NewValue;
}

bool FETransformComponent::IsSceneIndependent() const
{
	return bSceneIndependent;
}

void FETransformComponent::SetSceneIndependent(bool NewValue)
{
	bSceneIndependent = NewValue;
}