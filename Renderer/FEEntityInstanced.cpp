#include "FEEntityInstanced.h"

using namespace FocalEngine;

FEEntityInstanced::FEEntityInstanced(FEGameModel* gameModel, std::string Name) : FEEntity(gameModel, Name)
{
	type = FE_ENTITY_INSTANCED;
}

FEEntityInstanced::~FEEntityInstanced()
{
}

float FEEntityInstanced::render(glm::vec3 cameraPosition)
{
	if (instanceCount == 0)
		return 0.0f;

	if (lastFrameGameModel != gameModel)
	{
		dirtyFlag = true;
		lastFrameGameModel = gameModel;
	}

	if (dirtyFlag)
	{
		if (instancedBuffer != 0)
		{
			glDeleteBuffers(1, &instancedBuffer);
		}

		glGenBuffers(1, &instancedBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer);
		glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

		unsigned int VAO = gameModel->mesh->getVaoID();
		glBindVertexArray(VAO);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(9);
		glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(9, 1);

		glBindVertexArray(0);





		if (testLOD1 != nullptr)
		{
			unsigned int VAO = testLOD1->getVaoID();
			glBindVertexArray(VAO);
			// set attribute pointers for matrix (4 times vec4)
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(9);
			glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
			glVertexAttribDivisor(9, 1);

			glBindVertexArray(0);
		}
		





		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
		FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(glm::mat4), instancedMatrices.data()));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

		// force update AABB
		std::vector<glm::vec4> allPoints;
		allPoints.resize(8);

		FEAABB& meshAABB = gameModel->mesh->AABB;

		glm::vec4 bottomLeftFront = glm::vec4(meshAABB.min.x, meshAABB.min.y, meshAABB.max.z, 1.0f);
		glm::vec4 bottomRightFront = glm::vec4(meshAABB.max.x, meshAABB.min.y, meshAABB.max.z, 1.0f);
		glm::vec4 bottomRightBack = glm::vec4(meshAABB.max.x, meshAABB.min.y, meshAABB.min.z, 1.0f);
		glm::vec4 bottomLeftBack = glm::vec4(meshAABB.min.x, meshAABB.min.y, meshAABB.min.z, 1.0f);

		glm::vec4 topLeftFront = glm::vec4(meshAABB.min.x, meshAABB.max.y, meshAABB.max.z, 1.0f);
		glm::vec4 topRightFront = glm::vec4(meshAABB.max.x, meshAABB.max.y, meshAABB.max.z, 1.0f);
		glm::vec4 topRightBack = glm::vec4(meshAABB.max.x, meshAABB.max.y, meshAABB.min.z, 1.0f);
		glm::vec4 topLeftBack = glm::vec4(meshAABB.min.x, meshAABB.max.y, meshAABB.min.z, 1.0f);

		allInstancesAABB.min = glm::vec3(FLT_MAX);
		allInstancesAABB.max = glm::vec3(-FLT_MAX);

		for (size_t i = 0; i < instanceCount; i++)
		{
			// transform each point of this cube
			allPoints[0] = instancedMatrices[i] * bottomLeftFront;
			allPoints[1] = instancedMatrices[i] * bottomRightFront;
			allPoints[2] = instancedMatrices[i] * bottomRightBack;
			allPoints[3] = instancedMatrices[i] * bottomLeftBack;

			allPoints[4] = instancedMatrices[i] * topLeftFront;
			allPoints[5] = instancedMatrices[i] * topRightFront;
			allPoints[6] = instancedMatrices[i] * topRightBack;
			allPoints[7] = instancedMatrices[i] * topLeftBack;

			for (auto point : allPoints)
			{
				if (point.x < allInstancesAABB.min.x)
					allInstancesAABB.min.x = point.x;

				if (point.x > allInstancesAABB.max.x)
					allInstancesAABB.max.x = point.x;

				if (point.y < allInstancesAABB.min.y)
					allInstancesAABB.min.y = point.y;

				if (point.y > allInstancesAABB.max.y)
					allInstancesAABB.max.y = point.y;

				if (point.z < allInstancesAABB.min.z)
					allInstancesAABB.min.z = point.z;

				if (point.z > allInstancesAABB.max.z)
					allInstancesAABB.max.z = point.z;
			}
		}

		transform.dirtyFlag = true;
		getAABB();

		dirtyFlag = false;
	}

	float testTotalTime = 0.0f;
	if (testLOD0 != nullptr && testLOD1 != nullptr)
	{
		int testLOD0Count = 0;
		int testLOD1Count = 0;

		//auto start = std::chrono::system_clock::now();
		for (size_t i = 0; i < instanceCount; i++)
		{
			float distance = abs(instancedPositions[i].x - cameraPosition.x) + abs(instancedPositions[i].z - cameraPosition.z);
			/*alignas(16) int result[4] = { 0.0f };
			__m128i data = _mm_set_epi32(2, 3, 4 ,5);
			__m128i data1 = _mm_set_epi32(2, 3, 4, 5);
			
			
			data = _mm_add_epi32(data, data1);
			
			int y = data.m128i_i32[0];*/

			if (distance < 50)
			{
				testInstancedMatricesLOD0[testLOD0Count++] = instancedMatrices[i];
			}
			else if (cullDistance != -1 && cullDistance >= distance || cullDistance == -1)
			{
				testInstancedMatricesLOD1[testLOD1Count++] = instancedMatrices[i];
			}
		}

		//auto end = std::chrono::system_clock::now();
		//testTotalTime += std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() * 1000.0;

		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
		FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, testLOD0Count * sizeof(glm::mat4), testInstancedMatricesLOD0.data()));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

		FE_GL_ERROR(glBindVertexArray(testLOD0->getVaoID()));
		if ((gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glEnableVertexAttribArray(9));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, testLOD0->getVertexCount(), GL_UNSIGNED_INT, 0, testLOD0Count));
		FE_GL_ERROR(glBindVertexArray(0));


		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
		FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, testLOD1Count * sizeof(glm::mat4), testInstancedMatricesLOD1.data()));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

		FE_GL_ERROR(glBindVertexArray(testLOD1->getVaoID()));
		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, testLOD1->getVertexCount(), GL_UNSIGNED_INT, 0, testLOD1Count));
		FE_GL_ERROR(glBindVertexArray(0));
	}
	else if (cullDistance != -1)
	{
		int testLOD0Count = 0;
		//auto start = std::chrono::system_clock::now();
		for (size_t i = 0; i < instanceCount; i++)
		{
			float distance = abs(instancedPositions[i].x - cameraPosition.x) + abs(instancedPositions[i].z - cameraPosition.z);

			if (cullDistance >= distance)
			{
				testInstancedMatricesLOD0[testLOD0Count++] = instancedMatrices[i];
			}
		}

		//auto end = std::chrono::system_clock::now();
		//testTotalTime += std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() * 1000.0;

		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
		FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, testLOD0Count * sizeof(glm::mat4), testInstancedMatricesLOD0.data()));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

		FE_GL_ERROR(glBindVertexArray(gameModel->mesh->getVaoID()));
		if ((gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glEnableVertexAttribArray(9));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0, testLOD0Count));
		FE_GL_ERROR(glBindVertexArray(0));
	}
	else
	{
		FE_GL_ERROR(glBindVertexArray(gameModel->mesh->getVaoID()));
		if ((gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glEnableVertexAttribArray(9));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0, instanceCount));

		FE_GL_ERROR(glBindVertexArray(0));
	}

	return testTotalTime;
}

void FEEntityInstanced::clear()
{
	instancedMatrices.clear();
	instanceCount = 0;
}

void FEEntityInstanced::addInstance(glm::mat4 instanceMatrix)
{
	instancedMatrices.push_back(instanceMatrix);
	instancedPositions.push_back(glm::vec3(instanceMatrix[3][0], instanceMatrix[3][1], instanceMatrix[3][2]));
	instanceCount++;

	testInstancedMatricesLOD0.resize(instanceCount);
	testInstancedMatricesLOD1.resize(instanceCount);

	dirtyFlag = true;
}

FEAABB FEEntityInstanced::getAABB()
{
	if (transform.dirtyFlag)
	{
		// finally we apply whole instance matrix
		glm::vec4 bottomLeftFront = glm::vec4(allInstancesAABB.min.x, allInstancesAABB.min.y, allInstancesAABB.max.z, 1.0f);
		glm::vec4 bottomRightFront = glm::vec4(allInstancesAABB.max.x, allInstancesAABB.min.y, allInstancesAABB.max.z, 1.0f);
		glm::vec4 bottomRightBack = glm::vec4(allInstancesAABB.max.x, allInstancesAABB.min.y, allInstancesAABB.min.z, 1.0f);
		glm::vec4 bottomLeftBack = glm::vec4(allInstancesAABB.min.x, allInstancesAABB.min.y, allInstancesAABB.min.z, 1.0f);
		
		glm::vec4 topLeftFront = glm::vec4(allInstancesAABB.min.x, allInstancesAABB.max.y, allInstancesAABB.max.z, 1.0f);
		glm::vec4 topRightFront = glm::vec4(allInstancesAABB.max.x, allInstancesAABB.max.y, allInstancesAABB.max.z, 1.0f);
		glm::vec4 topRightBack = glm::vec4(allInstancesAABB.max.x, allInstancesAABB.max.y, allInstancesAABB.min.z, 1.0f);
		glm::vec4 topLeftBack = glm::vec4(allInstancesAABB.min.x, allInstancesAABB.max.y, allInstancesAABB.min.z, 1.0f);

		std::vector<glm::vec4> allPoints;
		allPoints.resize(8);

		allPoints[0] = transform.getTransformMatrix() * bottomLeftFront;
		allPoints[1] = transform.getTransformMatrix() * bottomRightFront;
		allPoints[2] = transform.getTransformMatrix() * bottomRightBack;
		allPoints[3] = transform.getTransformMatrix() * bottomLeftBack;

		allPoints[4] = transform.getTransformMatrix() * topLeftFront;
		allPoints[5] = transform.getTransformMatrix() * topRightFront;
		allPoints[6] = transform.getTransformMatrix() * topRightBack;
		allPoints[7] = transform.getTransformMatrix() * topLeftBack;

		entityAABB.min = glm::vec3(FLT_MAX);
		entityAABB.max = glm::vec3(-FLT_MAX);

		for (size_t i = 0; i < 8; i++)
		{
			if (allPoints[i].x < entityAABB.min.x)
				entityAABB.min.x = allPoints[i].x;

			if (allPoints[i].x > entityAABB.max.x)
				entityAABB.max.x = allPoints[i].x;

			if (allPoints[i].y < entityAABB.min.y)
				entityAABB.min.y = allPoints[i].y;

			if (allPoints[i].y > entityAABB.max.y)
				entityAABB.max.y = allPoints[i].y;

			if (allPoints[i].z < entityAABB.min.z)
				entityAABB.min.z = allPoints[i].z;

			if (allPoints[i].z > entityAABB.max.z)
				entityAABB.max.z = allPoints[i].z;
		}

		transform.dirtyFlag = false;
	}

	return entityAABB;
}

int FEEntityInstanced::getInstanceCount()
{
	return instanceCount;
}