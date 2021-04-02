#include "FEEntityInstanced.h"

using namespace FocalEngine;

float FESpawnInfo::getPositionDeviation()
{
	return (rand() % int(radius * 100)) / 100.0f - radius / 2.0f;
}

float FESpawnInfo::getScaleDeviation()
{
	float finalDeviation = ((float(rand() % int(scaleDeviation * 10000)) / 10000.0f) - scaleDeviation / 2.0f);
	/*if (finalDeviation < 0.0f)
		finalDeviation = 0.0f;*/
	return finalDeviation;
}

int FESpawnInfo::getRotaionDeviation(glm::vec3 axis)
{
	if (axis.x > 0.0f)
	{
		int rot = int(360 * rotationDeviation.x);
		if (rot == 0)
			return 0;
		return rand() % rot;
	}
	else if (axis.y > 0.0f)
	{
		int rot = int(360 * rotationDeviation.y);
		if (rot == 0)
			return 0;
		return rand() % rot;
	}
	else
	{
		int rot = int(360 * rotationDeviation.z);
		if (rot == 0)
			return 0;
		return rand() % rot;
	}
	
	return 0;
}

FEInstanceModification::FEInstanceModification()
{

}

FEEntityInstanced::FEEntityInstanced(FEGameModel* gameModel, std::string Name) : FEEntity(gameModel, Name)
{
	setType(FE_ENTITY_INSTANCED);

	instancedX = (float*)_aligned_malloc(sizeof(float) * 32, 32);
	instancedY = (float*)_aligned_malloc(sizeof(float) * 32, 32);
	instancedZ = (float*)_aligned_malloc(sizeof(float) * 32, 32);
	instancedXYZCount = 32;

	LODCounts = new int[gameModel->getMaxLODCount()];
	instancedMatricesLOD.resize(gameModel->getMaxLODCount());
	transform.setScale(glm::vec3(1.0f));
}

FEEntityInstanced::~FEEntityInstanced()
{
	_aligned_free(instancedX);
	_aligned_free(instancedY);
	_aligned_free(instancedZ);

	delete[] LODCounts;
}

void FEEntityInstanced::render()
{
	if (instanceCount == 0)
		return;

	if (lastFrameGameModel != gameModel || gameModel->dirtyFlag)
	{
		dirtyFlag = true;
		lastFrameGameModel = gameModel;
	}

	if (dirtyFlag)
	{
		updateBuffers();
		dirtyFlag = false;
		gameModel->dirtyFlag = false;
	}

	if (transform.dirtyFlag)
	{
		updateMatrices();
	}

	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		if (gameModel->isLODBillboard(i))
			break;

		if (LODCounts[i] > 0 && gameModel->getLODMesh(i) != nullptr)
		{
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
			FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, LODCounts[i] * sizeof(glm::mat4), instancedMatricesLOD[i].data()));

			FE_GL_ERROR(glBindVertexArray(gameModel->getLODMesh(i)->getVaoID()));

			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, gameModel->getLODMesh(i)->getVertexCount(), GL_UNSIGNED_INT, 0, LODCounts[i]));
			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}
	}

	if (cullingType == FE_CULLING_NONE)
	{
		FE_GL_ERROR(glBindVertexArray(gameModel->mesh->getVaoID()));
		if ((gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(9));
		FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0, instanceCount));

		FE_GL_ERROR(glBindVertexArray(0));
	}
}

void FEEntityInstanced::renderOnlyBillbords(glm::vec3 cameraPosition)
{
	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		if (gameModel->isLODBillboard(i) && gameModel->getLODMesh(i) != nullptr && LODCounts[i] > 0)
		{
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
			FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, LODCounts[i] * sizeof(glm::mat4), instancedMatricesBillboard.data()));

			FE_GL_ERROR(glBindVertexArray(gameModel->getLODMesh(i)->getVaoID()));

			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((gameModel->getLODMesh(i)->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, gameModel->getLODMesh(i)->getVertexCount(), GL_UNSIGNED_INT, 0, LODCounts[i]));

			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

			break;
		}
	}
}

void FEEntityInstanced::updateBuffers()
{
	if (instancedBuffer != 0)
	{
		glDeleteBuffers(1, &instancedBuffer);
	}
	
	FE_GL_ERROR(glGenBuffers(1, &instancedBuffer));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedBuffer));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), instancedMatrices.data(), GL_DYNAMIC_DRAW));

	unsigned int VAO = gameModel->mesh->getVaoID();
	FE_GL_ERROR(glBindVertexArray(VAO));
	// set attribute pointers for matrix (4 times vec4)
	FE_GL_ERROR(glEnableVertexAttribArray(6));
	FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
	FE_GL_ERROR(glEnableVertexAttribArray(7));
	FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
	FE_GL_ERROR(glEnableVertexAttribArray(8));
	FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
	FE_GL_ERROR(glEnableVertexAttribArray(9));
	FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

	FE_GL_ERROR(glVertexAttribDivisor(6, 1));
	FE_GL_ERROR(glVertexAttribDivisor(7, 1));
	FE_GL_ERROR(glVertexAttribDivisor(8, 1));
	FE_GL_ERROR(glVertexAttribDivisor(9, 1));

	FE_GL_ERROR(glBindVertexArray(0));

	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		if (gameModel->getLODMesh(i) != nullptr)
		{
			unsigned int VAO = gameModel->getLODMesh(i)->getVaoID();
			FE_GL_ERROR(glBindVertexArray(VAO));
			// set attribute pointers for matrix (4 times vec4)
			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glVertexAttribDivisor(6, 1));
			FE_GL_ERROR(glVertexAttribDivisor(7, 1));
			FE_GL_ERROR(glVertexAttribDivisor(8, 1));
			FE_GL_ERROR(glVertexAttribDivisor(9, 1));

			FE_GL_ERROR(glBindVertexArray(0));
		}
	}

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
		glm::mat4 matWithoutTranslate = transformedInstancedMatrices[i];
		matWithoutTranslate[3][0] -= transform.position.x;
		matWithoutTranslate[3][1] -= transform.position.y;
		matWithoutTranslate[3][2] -= transform.position.z;

		// transform each point of this cube
		allPoints[0] = matWithoutTranslate * bottomLeftFront;
		allPoints[1] = matWithoutTranslate * bottomRightFront;
		allPoints[2] = matWithoutTranslate * bottomRightBack;
		allPoints[3] = matWithoutTranslate * bottomLeftBack;

		allPoints[4] = matWithoutTranslate * topLeftFront;
		allPoints[5] = matWithoutTranslate * topRightFront;
		allPoints[6] = matWithoutTranslate * topRightBack;
		allPoints[7] = matWithoutTranslate * topLeftBack;

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
}

void FEEntityInstanced::clear()
{
	_aligned_free(instancedX);
	_aligned_free(instancedY);
	_aligned_free(instancedZ);

	delete[] LODCounts;

	instancedAABBSizes.resize(0);
	instancedMatrices.resize(0);
	transformedInstancedMatrices.resize(0);
	instanceCount = 0;

	instancedX = (float*)_aligned_malloc(sizeof(float) * 32, 32);
	instancedY = (float*)_aligned_malloc(sizeof(float) * 32, 32);
	instancedZ = (float*)_aligned_malloc(sizeof(float) * 32, 32);
	instancedXYZCount = 32;

	LODCounts = new int[gameModel->getMaxLODCount()];
	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		LODCounts[i] = 0;
	}

	instancedMatricesLOD.resize(gameModel->getMaxLODCount());
	transform.setScale(glm::vec3(1.0f));

	modifications.clear();
}

void FEEntityInstanced::addInstanceInternal(glm::mat4 instanceMatrix)
{
	instancedAABBSizes.push_back(-FEAABB(gameModel->getMesh()->getAABB(), instanceMatrix).size);
	instancedMatrices.push_back(instanceMatrix);
	transformedInstancedMatrices.push_back(transform.transformMatrix * instanceMatrix);

	if (instancedMatrices.size() >= instancedXYZCount)
	{
		instancedXYZCount = instancedMatrices.size() * 2;
		float* tempArrayX = (float*)_aligned_malloc(sizeof(float) * instancedXYZCount, 32);
		float* tempArrayY = (float*)_aligned_malloc(sizeof(float) * instancedXYZCount, 32);
		float* tempArrayZ = (float*)_aligned_malloc(sizeof(float) * instancedXYZCount, 32);

		for (size_t i = 0; i < instancedMatrices.size(); i++)
		{
			tempArrayX[i] = instancedX[i];
			tempArrayY[i] = instancedY[i];
			tempArrayZ[i] = instancedZ[i];
		}

		_aligned_free(instancedX);
		instancedX = tempArrayX;

		_aligned_free(instancedY);
		instancedY = tempArrayY;

		_aligned_free(instancedZ);
		instancedZ = tempArrayZ;
	}

	instancedX[instanceCount] = transformedInstancedMatrices.back()[3][0];
	instancedY[instanceCount] = transformedInstancedMatrices.back()[3][1];
	instancedZ[instanceCount] = transformedInstancedMatrices.back()[3][2];

	instanceCount++;

	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		instancedMatricesLOD[i].resize(instanceCount);
	}

	dirtyFlag = true;
}

void FEEntityInstanced::addInstances(glm::mat4* instanceMatrix, size_t count)
{
	size_t startIndex = instancedAABBSizes.size();

	instancedAABBSizes.resize(instancedAABBSizes.size() + count);
	FEAABB originalAABB = gameModel->getMesh()->getAABB();
	instancedMatrices.resize(instancedMatrices.size() + count);
	transformedInstancedMatrices.resize(transformedInstancedMatrices.size() + count);

	for (size_t i = startIndex; i < count; i++)
	{
		instancedAABBSizes[i] = -FEAABB(originalAABB, instanceMatrix[i]).size;
		instancedMatrices[i] = instanceMatrix[i];
		transformedInstancedMatrices[i] = transform.transformMatrix * instanceMatrix[i];
	}

	if (instancedMatrices.size() >= instancedXYZCount)
	{
		float* tempArrayX = (float*)_aligned_malloc(sizeof(float) * instancedMatrices.size() * 2, 32);
		float* tempArrayY = (float*)_aligned_malloc(sizeof(float) * instancedMatrices.size() * 2, 32);
		float* tempArrayZ = (float*)_aligned_malloc(sizeof(float) * instancedMatrices.size() * 2, 32);

		for (size_t i = 0; i < instancedXYZCount/*instancedMatrices.size()*/; i++)
		{
			tempArrayX[i] = instancedX[i];
			tempArrayY[i] = instancedY[i];
			tempArrayZ[i] = instancedZ[i];
		}
		instancedXYZCount = instancedMatrices.size() * 2;

		_aligned_free(instancedX);
		instancedX = tempArrayX;

		_aligned_free(instancedY);
		instancedY = tempArrayY;

		_aligned_free(instancedZ);
		instancedZ = tempArrayZ;
	}

	for (size_t i = startIndex; i < count; i++)
	{
		instancedX[instanceCount] = transformedInstancedMatrices[i][3][0];
		instancedY[instanceCount] = transformedInstancedMatrices[i][3][1];
		instancedZ[instanceCount] = transformedInstancedMatrices[i][3][2];

		instanceCount++;
	}

	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		instancedMatricesLOD[i].resize(instanceCount);
	}

	dirtyFlag = true;
}

FEAABB FEEntityInstanced::getAABB()
{
	if (transform.dirtyFlag)
	{
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

		updateMatrices();
		transform.dirtyFlag = false;
	}

	return entityAABB;
}

int FEEntityInstanced::getInstanceCount()
{
	return instanceCount;
}

float FEEntityInstanced::cullInstances(glm::vec3 cameraPosition, float** frustum, bool freezeCulling)
{
	float cullDistance = gameModel->cullDistance;
	float LOD0MAXDistance = gameModel->getLODMaxDrawDistance(0);
	float LOD1MAXDistance = gameModel->getLODMaxDrawDistance(1);
	float LOD2MAXDistance = gameModel->getLODMaxDrawDistance(2);

	if (frustum == nullptr || freezeCulling)
		return 0.0f;

	auto start = std::chrono::system_clock::now();

	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		LODCounts[i] = 0;
	}

#ifdef FE_SIMD_ENABLED
	float AABBSize = 0.0f;
	__m256 cullDistance256 = _mm256_set1_ps(cullDistance);

	__m256 frustum_00 = _mm256_set1_ps(frustum[0][0]);
	__m256 frustum_01 = _mm256_set1_ps(frustum[0][1]);
	__m256 frustum_02 = _mm256_set1_ps(frustum[0][2]);
	__m256 frustum_03 = _mm256_set1_ps(frustum[0][3]);

	__m256 frustum_10 = _mm256_set1_ps(frustum[1][0]);
	__m256 frustum_11 = _mm256_set1_ps(frustum[1][1]);
	__m256 frustum_12 = _mm256_set1_ps(frustum[1][2]);
	__m256 frustum_13 = _mm256_set1_ps(frustum[1][3]);

	__m256 frustum_20 = _mm256_set1_ps(frustum[2][0]);
	__m256 frustum_21 = _mm256_set1_ps(frustum[2][1]);
	__m256 frustum_22 = _mm256_set1_ps(frustum[2][2]);
	__m256 frustum_23 = _mm256_set1_ps(frustum[2][3]);

	__m256 frustum_30 = _mm256_set1_ps(frustum[3][0]);
	__m256 frustum_31 = _mm256_set1_ps(frustum[3][1]);
	__m256 frustum_32 = _mm256_set1_ps(frustum[3][2]);
	__m256 frustum_33 = _mm256_set1_ps(frustum[3][3]);

	__m256 frustum_40 = _mm256_set1_ps(frustum[4][0]);
	__m256 frustum_41 = _mm256_set1_ps(frustum[4][1]);
	__m256 frustum_42 = _mm256_set1_ps(frustum[4][2]);
	__m256 frustum_43 = _mm256_set1_ps(frustum[4][3]);

	__m256 frustum_50 = _mm256_set1_ps(frustum[5][0]);
	__m256 frustum_51 = _mm256_set1_ps(frustum[5][1]);
	__m256 frustum_52 = _mm256_set1_ps(frustum[5][2]);
	__m256 frustum_53 = _mm256_set1_ps(frustum[5][3]);

	size_t count = instanceCount;
	if (instanceCount % 8 != 0)
	{
		count = instanceCount - instanceCount % 8;
	}

	if (cullingType == FE_CULLING_LODS)
	{
		for (size_t i = 0; i < count; i+=8)
		{
			__m256 instancedPositionX = _mm256_load_ps(instancedX + i);
			__m256 instancedPositionY = _mm256_load_ps(instancedY + i);
			__m256 instancedPositionZ = _mm256_load_ps(instancedZ + i);

			// distance_0
			__m256 final_distance_0 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustum_00, instancedPositionX), _mm256_mul_ps(frustum_01, instancedPositionY)),
				_mm256_add_ps(_mm256_mul_ps(frustum_02, instancedPositionZ), frustum_03));

			// distance_1
			__m256 final_distance_1 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustum_10, instancedPositionX), _mm256_mul_ps(frustum_11, instancedPositionY)),
				_mm256_add_ps(_mm256_mul_ps(frustum_12, instancedPositionZ), frustum_13));

			// distance_2
			__m256 final_distance_2 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustum_20, instancedPositionX), _mm256_mul_ps(frustum_21, instancedPositionY)),
				_mm256_add_ps(_mm256_mul_ps(frustum_22, instancedPositionZ), frustum_23));

			// distance_3
			__m256 final_distance_3 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustum_30, instancedPositionX), _mm256_mul_ps(frustum_31, instancedPositionY)),
				_mm256_add_ps(_mm256_mul_ps(frustum_32, instancedPositionZ), frustum_33));

			// distance_4
			__m256 final_distance_4 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustum_40, instancedPositionX), _mm256_mul_ps(frustum_41, instancedPositionY)),
				_mm256_add_ps(_mm256_mul_ps(frustum_42, instancedPositionZ), frustum_43));

			// distance_5
			__m256 final_distance_5 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(frustum_50, instancedPositionX), _mm256_mul_ps(frustum_51, instancedPositionY)),
				_mm256_add_ps(_mm256_mul_ps(frustum_52, instancedPositionZ), frustum_53));

			// all 8 AABB should be culled, skiping this batch
			if (_mm256_movemask_ps(_mm256_cmp_ps(final_distance_5, cullDistance256, _CMP_GT_OS)) == 255)
				continue;

			// final_distance_5.m256_f32 > AABBSize is commented because it is not making a lot of difference(if near plane is very close to camera)
			// but in case of rendering shadow maps I use light for frustum culling but main camera near plane for LODs and distance culling
			// so I should not use main camera near plane for frustum culling(shadow map could include stuff behind main camera)

			// check 0 AABB
			AABBSize = instancedAABBSizes[i];
			if (cullDistance >= final_distance_5.m256_f32[0])
			{
				
				if (final_distance_0.m256_f32[0] > AABBSize && final_distance_1.m256_f32[0] > AABBSize &&
					final_distance_2.m256_f32[0] > AABBSize && final_distance_3.m256_f32[0] > AABBSize &&
					final_distance_4.m256_f32[0] > AABBSize /*&& final_distance_5.m256_f32[0] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[0] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i];
					}
					else if (final_distance_5.m256_f32[0] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i];
					}
					else if (final_distance_5.m256_f32[0] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i];
					}
				}
			}

			// check 1 AABB
			AABBSize = instancedAABBSizes[i + 1];
			if (cullDistance >= final_distance_5.m256_f32[1])
			{
				if (final_distance_0.m256_f32[1] > AABBSize && final_distance_1.m256_f32[1] > AABBSize &&
					final_distance_2.m256_f32[1] > AABBSize && final_distance_3.m256_f32[1] > AABBSize &&
					final_distance_4.m256_f32[1] > AABBSize /*&& final_distance_5.m256_f32[1] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[1] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 1];
					}
					else if (final_distance_5.m256_f32[1] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 1];
					}
					else if (final_distance_5.m256_f32[1] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 1];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 1];
					}
				}
			}

			// check 2 AABB
			AABBSize = instancedAABBSizes[i + 2];
			if (cullDistance >= final_distance_5.m256_f32[2])
			{
				if (final_distance_0.m256_f32[2] > AABBSize && final_distance_1.m256_f32[2] > AABBSize &&
					final_distance_2.m256_f32[2] > AABBSize && final_distance_3.m256_f32[2] > AABBSize &&
					final_distance_4.m256_f32[2] > AABBSize /*&& final_distance_5.m256_f32[2] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[2] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 2];
					}
					else if (final_distance_5.m256_f32[2] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 2];
					}
					else if (final_distance_5.m256_f32[2] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 2];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 2];
					}
				}
			}

			// check 3 AABB
			AABBSize = instancedAABBSizes[i + 3];
			if (cullDistance >= final_distance_5.m256_f32[3])
			{
				if (final_distance_0.m256_f32[3] > AABBSize && final_distance_1.m256_f32[3] > AABBSize &&
					final_distance_2.m256_f32[3] > AABBSize && final_distance_3.m256_f32[3] > AABBSize &&
					final_distance_4.m256_f32[3] > AABBSize /*&& final_distance_5.m256_f32[3] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[3] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 3];
					}
					else if (final_distance_5.m256_f32[3] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 3];
					}
					else if (final_distance_5.m256_f32[3] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 3];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 3];
					}
				}
			}

			// check 4 AABB
			AABBSize = instancedAABBSizes[i + 4];
			if (cullDistance >= final_distance_5.m256_f32[4])
			{
				if (final_distance_0.m256_f32[4] > AABBSize && final_distance_1.m256_f32[4] > AABBSize &&
					final_distance_2.m256_f32[4] > AABBSize && final_distance_3.m256_f32[4] > AABBSize &&
					final_distance_4.m256_f32[4] > AABBSize /*&& final_distance_5.m256_f32[4] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[4] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 4];
					}
					else if (final_distance_5.m256_f32[4] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 4];
					}
					else if (final_distance_5.m256_f32[4] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 4];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 4];
					}
				}
			}

			// check 5 AABB
			AABBSize = instancedAABBSizes[i + 5];
			if (cullDistance >= final_distance_5.m256_f32[5])
			{
				if (final_distance_0.m256_f32[5] > AABBSize && final_distance_1.m256_f32[5] > AABBSize &&
					final_distance_2.m256_f32[5] > AABBSize && final_distance_3.m256_f32[5] > AABBSize &&
					final_distance_4.m256_f32[5] > AABBSize /*&& final_distance_5.m256_f32[5] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[5] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 5];
					}
					else if (final_distance_5.m256_f32[5] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 5];
					}
					else if (final_distance_5.m256_f32[5] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 5];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 5];
					}
				}
			}

			// check 6 AABB
			AABBSize = instancedAABBSizes[i + 6];
			if (cullDistance >= final_distance_5.m256_f32[6])
			{
				if (final_distance_0.m256_f32[6] > AABBSize && final_distance_1.m256_f32[6] > AABBSize &&
					final_distance_2.m256_f32[6] > AABBSize && final_distance_3.m256_f32[6] > AABBSize &&
					final_distance_4.m256_f32[6] > AABBSize /*&& final_distance_5.m256_f32[6] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[6] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 6];
					}
					else if (final_distance_5.m256_f32[6] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 6];
					}
					else if (final_distance_5.m256_f32[6] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 6];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 6];
					}
				}
			}

			// check 7 AABB
			AABBSize = instancedAABBSizes[i + 7];
			if (cullDistance >= final_distance_5.m256_f32[7])
			{
				if (final_distance_0.m256_f32[7] > AABBSize && final_distance_1.m256_f32[7] > AABBSize &&
					final_distance_2.m256_f32[7] > AABBSize && final_distance_3.m256_f32[7] > AABBSize &&
					final_distance_4.m256_f32[7] > AABBSize /*&& final_distance_5.m256_f32[7] > AABBSize*/)
				{
					if (final_distance_5.m256_f32[7] > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i + 7];
					}
					else if (final_distance_5.m256_f32[7] > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i + 7];
					}
					else if (final_distance_5.m256_f32[7] > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i + 7];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i + 7];
					}
				}
			}
		}

		// in order to use SIMD we need 8 instances at a time but if we have left over we should use non-SIMD way.
		if (instanceCount % 8 != 0)
		{
			for (size_t i = instanceCount - instanceCount % 8; i < instanceCount; i++)
			{
				float distance;
				for (int p = 0; p < 6; p++)
				{
					distance = frustum[p][0] * instancedX[i] + frustum[p][1] * instancedY[i] + frustum[p][2] * instancedZ[i] + frustum[p][3];
					if (distance <= instancedAABBSizes[i])
						break;
				}

				if (distance > instancedAABBSizes[i] && cullDistance >= distance)
				{
					if (distance > LOD2MAXDistance)
					{
						instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i];
					}
					else if (distance > LOD1MAXDistance)
					{
						instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i];
					}
					else if (distance > LOD0MAXDistance)
					{
						instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i];
					}
					else
					{
						instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i];
					}
				}
			}
		}
	}
#else
	// Non SIMD Variant
	for (size_t i = 0; i < instanceCount; i++)
	{
		float distance;
		for (int p = 0; p < 6; p++)
		{
			distance = frustum[p][0] * instancedX[i] + frustum[p][1] * instancedY[i] + frustum[p][2] * instancedZ[i] + frustum[p][3];
			if (distance <= instancedAABBSizes[i])
				break;
		}

		if (distance > instancedAABBSizes[i] && cullDistance >= distance)
		{
			if (distance > LOD2MAXDistance)
			{
				instancedMatricesLOD[3][LODCounts[3]++] = transformedInstancedMatrices[i];
			}
			else if (distance > LOD1MAXDistance)
			{
				instancedMatricesLOD[2][LODCounts[2]++] = transformedInstancedMatrices[i];
			}
			else if (distance > LOD0MAXDistance)
			{
				instancedMatricesLOD[1][LODCounts[1]++] = transformedInstancedMatrices[i];
			}
			else
			{
				instancedMatricesLOD[0][LODCounts[0]++] = transformedInstancedMatrices[i];
			}
		}
	}
#endif // FE_SIMD_ENABLED


	// Billboard set up
	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		if (gameModel->isLODBillboard(i) && gameModel->getLODMesh(i) != nullptr && LODCounts[i] > 0)
		{
			if (instancedMatricesBillboard.size() != instancedMatricesLOD[i].size())
			{
				instancedMatricesBillboard.resize(instancedMatricesLOD[i].size());
			}

			for (int j = 0; j < LODCounts[i]; j++)
			{
				float xDiff = cameraPosition.x - instancedMatricesLOD[i][j][3][0];
				float yDiff = cameraPosition.z - instancedMatricesLOD[i][j][3][2];

				float angle = atan2(yDiff, xDiff);
				// this should not be here, instead normal of plane should align with vector to camera
				angle += 1.5708f * 3.0f + gameModel->getBillboardZeroRotaion() * ANGLE_TORADIANS_COF;
				angle = -angle;

				instancedMatricesBillboard[j] = glm::mat4(1.0);
				instancedMatricesBillboard[j] = glm::translate(instancedMatricesBillboard[j], glm::vec3(instancedMatricesLOD[i][j][3]));
				instancedMatricesBillboard[j] = glm::rotate(instancedMatricesBillboard[j], angle, glm::vec3(0, 1, 0));


				float extractedScale = glm::length(instancedMatricesLOD[i][j][0]);
				instancedMatricesBillboard[j] = glm::scale(instancedMatricesBillboard[j], glm::vec3(extractedScale/*gameModel->getScaleFactor()*/));

				//instancedMatricesBillboard[j] = glm::mat4(1.0);
				//// position
				//instancedMatricesBillboard[j][3][0] = instancedMatricesLOD[i][j][3][0];
				//instancedMatricesBillboard[j][3][1] = instancedMatricesLOD[i][j][3][1];
				//instancedMatricesBillboard[j][3][2] = instancedMatricesLOD[i][j][3][2];

				//// rotation
				//instancedMatricesBillboard[j][0][0] = cos(angle);
				//instancedMatricesBillboard[j][2][0] = sin(angle);

				//instancedMatricesBillboard[j][0][2] = -sin(angle);
				//instancedMatricesBillboard[j][2][2] = cos(angle);

				//// scale
				//instancedMatricesBillboard[j][0][0] *= gameModel->getBillboardScale();
				//instancedMatricesBillboard[j][1][1] *= gameModel->getBillboardScale();
				//instancedMatricesBillboard[j][2][2] *= gameModel->getBillboardScale();
			}
		}
	}

	auto end = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::duration<float>>(end - start).count() * 1000.0f;
}

void FEEntityInstanced::updateMatrices()
{
	if (instancedMatrices.size() != transformedInstancedMatrices.size())
	{
		LOG.logError("instancedMatrices size and transformedInstancedMatrices size is not equal!");
		return;
	}

	for (size_t i = 0; i < instancedMatrices.size(); i++)
	{
		transformedInstancedMatrices[i] = transform.transformMatrix * instancedMatrices[i];
		instancedX[i] = transformedInstancedMatrices[i][3][0];
		instancedY[i] = transformedInstancedMatrices[i][3][1];
		instancedZ[i] = transformedInstancedMatrices[i][3][2];
	}
}

bool FEEntityInstanced::populate(FESpawnInfo spawnInfo)
{
	if (spawnInfo.radius <= 0.0f || spawnInfo.objectCount < 1 || spawnInfo.objectCount > 1000000 || gameModel == nullptr)
		return false;

	this->spawnInfo = spawnInfo;
	srand(spawnInfo.seed);

	glm::vec3 min = gameModel->getMesh()->getAABB().getMin();
	glm::vec3 max = gameModel->getMesh()->getAABB().getMax();

	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	ySize *= gameModel->getScaleFactor();

	std::vector<glm::mat4> newMats;
	newMats.resize(spawnInfo.objectCount);

	for (size_t i = 0; i < newMats.size(); i++)
	{
		glm::mat4 newMat = glm::mat4(1.0);
		// spawner transformation would be taken in account later so consider center in 0
		float x = spawnInfo.getPositionDeviation();
		float z = spawnInfo.getPositionDeviation();
		float y = spawnInfo.getPositionDeviation();

		if (terrainToSnap != nullptr)
		{
			y = std::invoke(getTerrainY, terrainToSnap, glm::vec2(transform.position.x + x, transform.position.z + z));

			int countOfTries = 0;
			while (y == -FLT_MAX)
			{
				x = spawnInfo.getPositionDeviation();
				z = spawnInfo.getPositionDeviation();
				y = std::invoke(getTerrainY, terrainToSnap, glm::vec2(transform.position.x + x, transform.position.z + z));
				countOfTries++;
				if (countOfTries > 100)
					break;
			}

			if (countOfTries > 100)
			{
				y = transform.position.y + spawnInfo.getPositionDeviation();
			}
		}

		newMat = glm::translate(newMat, glm::vec3(x, y, z));

		newMat = glm::rotate(newMat, spawnInfo.getRotaionDeviation(glm::vec3(1, 0, 0)) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
		newMat = glm::rotate(newMat, spawnInfo.getRotaionDeviation(glm::vec3(0, 1, 0)) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
		newMat = glm::rotate(newMat, spawnInfo.getRotaionDeviation(glm::vec3(0, 0, 1)) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

		float finalScale = gameModel->getScaleFactor() + gameModel->getScaleFactor() * spawnInfo.getScaleDeviation();
		if (finalScale < 0.0f)
			finalScale = 0.01f;
		newMat = glm::scale(newMat, glm::vec3(finalScale));

		newMats[i] = newMat;
	}
	addInstances(newMats.data(), newMats.size());

	if (terrainToSnap != nullptr)
	{
		// terrain.y could be not 0.0f but here we should indicate 0.0f as Y.
		transform.setPosition(glm::vec3(transform.position.x, 0.0f, transform.position.z));
	}

	srand(unsigned int(time(NULL)));

	return true;
}

FETerrain* FEEntityInstanced::getSnappedToTerrain()
{
	return terrainToSnap;
}

void FEEntityInstanced::updateSelectModeAABBData()
{
	instancedAABB.clear();
	instancedAABB.resize(transformedInstancedMatrices.size());

	for (size_t i = 0; i < transformedInstancedMatrices.size(); i++)
	{
		instancedAABB[i] = FEAABB(gameModel->getMesh()->getAABB(), transformedInstancedMatrices[i]);
	}
}

bool FEEntityInstanced::isSelectMode()
{
	return selectionMode;
}

void FEEntityInstanced::setSelectMode(bool newValue)
{
	if (newValue)
		updateSelectModeAABBData();
	
	selectionMode = newValue;
}

void FEEntityInstanced::deleteInstance(size_t instanceIndex)
{
	if (instanceIndex < 0 || instanceIndex >= instancedMatrices.size())
		return;

	modifications.push_back(FEInstanceModification(CHANGE_DELETED, instanceIndex, glm::mat4()));

	instancedAABBSizes.erase(instancedAABBSizes.begin() + instanceIndex);
	instancedMatrices.erase(instancedMatrices.begin() + instanceIndex);
	transformedInstancedMatrices.erase(transformedInstancedMatrices.begin() + instanceIndex);

	for (size_t i = instanceIndex; i < instancedXYZCount; i++)
	{
		instancedX[i] = instancedX[i + 1];
		instancedY[i] = instancedY[i + 1];
		instancedZ[i] = instancedZ[i + 1];
	}

	instancedXYZCount--;
	instanceCount--;

	for (size_t i = 0; i < gameModel->getMaxLODCount(); i++)
	{
		instancedMatricesLOD[i].resize(instanceCount);
	}

	if (instancedAABB.size() == 0)
	{
		updateSelectModeAABBData();
	}
	else
	{
		instancedAABB.erase(instancedAABB.begin() + instanceIndex);
	}
	
	//std::vector<glm::mat4> instancedMatricesCopy = instancedMatrices;
	//instancedMatricesCopy.erase(instancedMatricesCopy.begin() + instanceIndex);
	//clear(true);
	//addInstances(instancedMatricesCopy.data(), instancedMatricesCopy.size());

	//if (terrainToSnap != nullptr)
	//{
	//	// terrain.y could be not 0.0f but here we should indicate 0.0f as Y.
	//	transform.setPosition(glm::vec3(transform.position.x, 0.0f, transform.position.z));
	//}

	//testEnableSelectMode();
}

glm::mat4 FEEntityInstanced::getTransformedInstancedMatrix(size_t instanceIndex)
{
	if (instanceIndex < 0 || instanceIndex >= transformedInstancedMatrices.size())
		return glm::identity<glm::mat4>();

	return transformedInstancedMatrices[instanceIndex];
}

void FEEntityInstanced::modifyInstance(size_t instanceIndex, glm::mat4 newMatrix)
{
	if (instanceIndex < 0 || instanceIndex >= transformedInstancedMatrices.size())
		return;

	if (glm::all(glm::epsilonEqual(transformedInstancedMatrices[instanceIndex][0], newMatrix[0], 0.001f)) &&
		glm::all(glm::epsilonEqual(transformedInstancedMatrices[instanceIndex][1], newMatrix[1], 0.001f)) &&
		glm::all(glm::epsilonEqual(transformedInstancedMatrices[instanceIndex][2], newMatrix[2], 0.001f)) &&
		glm::all(glm::epsilonEqual(transformedInstancedMatrices[instanceIndex][3], newMatrix[3], 0.001f)))
		return;

	if (modifications.size() > 0 && modifications.back().index == instanceIndex && modifications.back().type == CHANGE_MODIFIED)
	{
		modifications.back().modification = newMatrix;
	}
	else
	{
		modifications.push_back(FEInstanceModification(CHANGE_MODIFIED, instanceIndex, newMatrix));
	}
	
	transformedInstancedMatrices[instanceIndex] = newMatrix;
	instancedMatrices[instanceIndex] = glm::inverse(transform.getTransformMatrix()) * newMatrix;

	if (instancedAABB.size() > instanceIndex)
		instancedAABB[instanceIndex] = FEAABB(gameModel->getMesh()->getAABB(), newMatrix);
	instancedAABBSizes[instanceIndex] = -FEAABB(gameModel->getMesh()->getAABB(), newMatrix).size;
}

int FEEntityInstanced::getSpawnModificationCount()
{
	return modifications.size();
}

std::vector<FEInstanceModification> FEEntityInstanced::getSpawnModifications()
{
	return modifications;
}

void FEEntityInstanced::addInstance(glm::mat4 instanceMatrix)
{
	addInstanceInternal(glm::inverse(transform.transformMatrix) * instanceMatrix);

	if (instancedAABB.size() == 0)
	{
		updateSelectModeAABBData();
	}
	else
	{
		instancedAABB.push_back(FEAABB(gameModel->getMesh()->getAABB(), transformedInstancedMatrices.back()));
	}

	modifications.push_back(FEInstanceModification(CHANGE_ADDED, instancedMatrices.size(), instanceMatrix));
}

bool FEEntityInstanced::tryToSnapInstance(size_t instanceIndex)
{
	if (instanceIndex < 0 || instanceIndex >= transformedInstancedMatrices.size() || terrainToSnap == nullptr)
		return false;

	if (!isSelectMode())
		return false;

	float y = std::invoke(getTerrainY, terrainToSnap, glm::vec2(transformedInstancedMatrices[instanceIndex][3][0], transformedInstancedMatrices[instanceIndex][3][2]));
	if (y == -FLT_MAX)
		return false;

	if (abs(transformedInstancedMatrices[instanceIndex][3][1] - y) < 0.01f)
		return true;

	glm::mat4 copy = transformedInstancedMatrices[instanceIndex];
	copy[3][1] = y;
	modifyInstance(instanceIndex, copy);
	return true;
}