#pragma once

#include "FECoreIncludes.h"

namespace FocalEngine
{
	class FEBasicCamera
	{
	public:
		FEBasicCamera();
		~FEBasicCamera();

		float getFov();
		void setFov(float newFov);
		float getNearPlane();
		void setNearPlane(float newNearPlane);
		float getFarPlane();
		void setFarPlane(float newFarPlane);

		float getYaw();
		void setYaw(float newYaw);
		float getPitch();
		void setPitch(float newPitch);
		float getRoll();
		void setRoll(float newRoll);
		glm::vec3 getPosition();
		void setPosition(glm::vec3 newPosition);

		float getAspectRatio();
		void setAspectRatio(float newAspectRatio);

		float getGamma();
		void setGamma(float newGamma);
		float getExposure();
		void setExposure(float newExposure);

		glm::mat4 getViewMatrix();
		virtual void updateViewMatrix();
		glm::mat4 getProjectionMatrix();
		virtual void updateProjectionMatrix();

		virtual void updateAll();

		virtual void move(float deltaTime = 0.0f) {};

		virtual void keyboardInput(int key, int scancode, int action, int mods) {};
		virtual void mouseMoveInput(double xpos, double ypos) {};
	protected:
		float fov = 70.0f;
		float nearPlane = 0.1f;
		float farPlane = 5000.0f;
		float aspectRatio = 1.0f;

		float yaw = 0.0f;
		float pitch = 0.0f;
		float roll = 0.0f;

		float gamma = 2.2f;
		float exposure = 1.0f;

		glm::vec3 position = glm::vec3(0.0f);
		glm::mat4 viewMatrix;
		glm::mat4 projectionMatrix;

		GLFWwindow* window;
	};
}