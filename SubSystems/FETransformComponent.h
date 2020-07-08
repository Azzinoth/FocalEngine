#pragma once
#include "FELog.h"

namespace FocalEngine
{
	class FETransformComponent
	{
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

		bool uniformScaling = true;
	private:
		glm::vec3 position;
		glm::quat rotationQuaternion;
		glm::vec3 rotationAngles;
		
		glm::vec3 scale;

		glm::mat4 transformMatrix;

		void rotateQuaternion(float angle, glm::vec3 axis);
	};
}