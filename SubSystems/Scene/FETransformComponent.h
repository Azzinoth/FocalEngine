#pragma once
#include "../Core/FEObject.h"

namespace FocalEngine
{
	class FEEntity;
	class FEEntityInstanced;
	class FETerrain;
	class FERenderer;

	class FETransformComponent
	{
		friend FEEntity;
		friend FEEntityInstanced;
		friend FETerrain;
		friend FERenderer;
	public:
		FETransformComponent();
		FETransformComponent(glm::mat4 Matrix);
		~FETransformComponent();

		FETransformComponent Combine(const FETransformComponent& Other) const;

		glm::vec3 GetPosition() const;
		glm::vec3 GetRotation() const;
		glm::quat GetQuaternion() const;
		glm::vec3 GetScale() const;

		void SetPosition(glm::vec3 NewPosition);
		void SetRotation(glm::vec3 NewRotation);
		void SetQuaternion(glm::quat Quaternion);
		void RotateByQuaternion(glm::quat Quaternion);
		void SetScale(glm::vec3 NewScale);

		void ChangeScaleUniformlyBy(float Delta);
		void ChangeXScaleBy(float Delta);
		void ChangeYScaleBy(float Delta);
		void ChangeZScaleBy(float Delta);

		glm::mat4 GetTransformMatrix() const;
		void ForceSetTransformMatrix(glm::mat4 NewValue);
		void Update();

		bool IsDirty() const;
		void SetDirtyFlag(bool NewValue);

		bool IsUniformScalingSet() const;
		void SetUniformScaling(bool NewValue);

		int OldWayForce = -1;
		static bool OldWayGlobal;
		glm::mat4 WorldSpaceMatrix = glm::identity<glm::mat4>();
		glm::mat4 LocalSpaceMatrix = glm::identity<glm::mat4>();
		bool bUseLocalSpace = true;
		bool bIsInSceneGraph = false;
	private:
		bool bDirtyFlag = false;
		glm::vec3 Position;
		glm::quat RotationQuaternion;
		glm::vec3 RotationAngles;
		bool bUniformScaling = true;
		
		glm::vec3 Scale;

		glm::mat4 TransformMatrix;
		
		glm::mat4 PreviousTransformMatrix;

		void RotateQuaternion(float Angle, glm::vec3 Axis);
	};
}