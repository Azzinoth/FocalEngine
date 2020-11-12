#pragma once
#include "FELog.h"

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
		~FETransformComponent();

		glm::vec3 getPosition();
		glm::vec3 getRotation();
		glm::quat getQuaternion();
		glm::vec3 getScale();

		void setPosition(glm::vec3 newPosition);
		void setRotation(glm::vec3 newRotation);
		void rotateByQuaternion(glm::quat quaternion);
		void setScale(glm::vec3 newScale);

		void changeScaleUniformlyBy(float delta);
		void changeXScaleBy(float delta);
		void changeYScaleBy(float delta);
		void changeZScaleBy(float delta);

		glm::mat4 getTransformMatrix();
		void update();

		bool isDirty();
		void setDirty(bool isDirty);

		bool uniformScaling = true;
	private:
		bool dirtyFlag = false;
		glm::vec3 position;
		glm::quat rotationQuaternion;
		glm::vec3 rotationAngles;
		
		glm::vec3 scale;

		glm::mat4 transformMatrix;
		glm::mat4 previousTransformMatrix;

		void rotateQuaternion(float angle, glm::vec3 axis);
	};
}