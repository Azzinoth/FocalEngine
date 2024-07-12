#pragma once

#include "../Core/FEObject.h"
#include "../Core/FEGeometricTools.h"

namespace FocalEngine
{
	class FETransformComponent
	{
		friend class FEScene;
		friend class FERenderer;
	public:
		FETransformComponent();
		FETransformComponent(glm::mat4 Matrix);
		~FETransformComponent();

		FETransformComponent Combine(const FETransformComponent& Other) const;

		glm::vec3 GetPosition(bool bLocalSpace = true) const;
		glm::vec3 GetRotation(bool bLocalSpace = true) const;
		glm::quat GetQuaternion(bool bLocalSpace = true) const;
		glm::vec3 GetScale(bool bLocalSpace = true) const;

		void SetPosition(glm::vec3 NewPosition, bool bLocalSpace = true);
		void SetRotation(glm::vec3 NewRotation, bool bLocalSpace = true);
		void RotateAroundAxis(const glm::vec3& Axis, const float& RotationAmount, bool bLocalSpace = true);
		void SetQuaternion(glm::quat Quaternion, bool bLocalSpace = true);
		void RotateByQuaternion(glm::quat Quaternion);
		void SetScale(glm::vec3 NewScale, bool bLocalSpace = true);

		glm::mat4 GetWorldMatrix() const;
		// Use this function only if you know what you are doing
		// Usually, it is used to forcefully set the world matrix before the scene update occurs
		void ForceSetWorldMatrix(glm::mat4 NewMatrix);
		glm::mat4 GetLocalMatrix() const;
		// Use this function only if you know what you are doing
		void ForceSetLocalMatrix(glm::mat4 NewMatrix);

		void Update();

		bool IsDirty() const;
		void SetDirtyFlag(bool NewValue);

		bool IsUniformScalingSet() const;
		void SetUniformScaling(bool NewValue);

		bool IsSceneIndependent() const;
		void SetSceneIndependent(bool NewValue);
	private:
		bool bDirtyFlag = false;
		bool bSceneIndependent = false;

		glm::mat4 WorldSpaceMatrix = glm::identity<glm::mat4>();
		glm::mat4 LocalSpaceMatrix = glm::identity<glm::mat4>();

		glm::vec3 Position = glm::vec3(0.0f);

		glm::quat RotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
		glm::vec3 RotationAngles = glm::vec3(0.0f);

		bool bUniformScaling = true;
		glm::vec3 Scale = glm::vec3(1.0f);

		void RotateQuaternion(float Angle, glm::vec3 Axis);
		glm::mat4 GetParentMatrix() const;

		void MoveAlongAxis(const glm::vec3& Axis, float MovementValue, bool bLocalSpace = true);
	};
}