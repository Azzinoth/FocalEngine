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

FEEntityInstanced::FEEntityInstanced(FEPrefab* prefab, std::string Name) : FEEntity(prefab, Name)
{
	setType(FE_ENTITY_INSTANCED);

	renderers.resize(prefab->components.size());
	for (int i = 0; i < prefab->components.size(); i++)
	{
		initRender(i);
	}

	transform.setScale(glm::vec3(1.0f));
}

void FEEntityInstanced::initRender(int index)
{
	renderers[index] = new FEGameModelInstancedRenderer;
	renderers[index]->LODCounts = new int[prefab->components[index]->gameModel->getMaxLODCount()];

	renderers[index]->LODBuffers = new GLenum[prefab->components[index]->gameModel->getMaxLODCount()];
	for (size_t j = 0; j < prefab->components[index]->gameModel->getMaxLODCount(); j++)
	{
		FE_GL_ERROR(glGenBuffers(1, &renderers[index]->LODBuffers[j]));
	}

	FE_GL_ERROR(glGenBuffers(1, &renderers[index]->sourceDataBuffer));
	FE_GL_ERROR(glGenBuffers(1, &renderers[index]->positionsBuffer));
	FE_GL_ERROR(glGenBuffers(1, &renderers[index]->AABBSizesBuffer));
	FE_GL_ERROR(glGenBuffers(1, &renderers[index]->LODInfoBuffer));

	renderers[index]->indirectDrawsInfo = new FEDrawElementsIndirectCommand[4];
	for (size_t j = 0; j < prefab->components[index]->gameModel->getMaxLODCount(); j++)
	{
		renderers[index]->indirectDrawsInfo[j].count = prefab->components[index]->gameModel->getLODMesh(j) == nullptr ? 0 : prefab->components[index]->gameModel->getLODMesh(j)->getVertexCount();
		renderers[index]->indirectDrawsInfo[j].baseInstance = 0;
		renderers[index]->indirectDrawsInfo[j].baseVertex = 0;
		renderers[index]->indirectDrawsInfo[j].firstIndex = 0;
		renderers[index]->indirectDrawsInfo[j].primCount = 0;
	}

	FE_GL_ERROR(glGenBuffers(1, &renderers[index]->indirectDrawInfoBuffer));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderers[index]->indirectDrawInfoBuffer));
	FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, renderers[index]->indirectDrawsInfo, GL_MAP_READ_BIT));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

	renderers[index]->instancedMatricesLOD.resize(prefab->components[index]->gameModel->getMaxLODCount());
}

FEEntityInstanced::~FEEntityInstanced()
{
	delete[] renderers.back()->LODCounts;
	delete[] renderers.back()->indirectDrawsInfo;
}

void FEEntityInstanced::render(int subGameModel)
{
	if (instanceCount == 0)
		return;

	if (renderers.size() <= subGameModel || prefab->components.size() <= subGameModel)
		return;

	checkDirtyFlag(subGameModel);

	for (size_t i = 0; i < prefab->components[subGameModel]->gameModel->getMaxLODCount(); i++)
	{
		if (prefab->components[subGameModel]->gameModel->isLODBillboard(i))
			break;

		if (prefab->components[subGameModel]->gameModel->getLODMesh(i) != nullptr)
		{
			if (renderers[subGameModel]->LODBuffers[i] == 0)
				break;

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, renderers[subGameModel]->LODBuffers[i]));

			FE_GL_ERROR(glBindVertexArray(prefab->components[subGameModel]->gameModel->getLODMesh(i)->getVaoID()));

			if ((prefab->components[subGameModel]->gameModel->getLODMesh(i)->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((prefab->components[subGameModel]->gameModel->getLODMesh(i)->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((prefab->components[subGameModel]->gameModel->getLODMesh(i)->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((prefab->components[subGameModel]->gameModel->getLODMesh(i)->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((prefab->components[subGameModel]->gameModel->getLODMesh(i)->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((prefab->components[subGameModel]->gameModel->getLODMesh(i)->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderers[subGameModel]->indirectDrawInfoBuffer));
			FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(i * sizeof(FEDrawElementsIndirectCommand))));
			
			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}
	}

	if (renderers[subGameModel]->cullingType == FE_CULLING_NONE)
	{
		FE_GL_ERROR(glBindVertexArray(prefab->components[subGameModel]->gameModel->mesh->getVaoID()));
		if ((prefab->components[subGameModel]->gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((prefab->components[subGameModel]->gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((prefab->components[subGameModel]->gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((prefab->components[subGameModel]->gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((prefab->components[subGameModel]->gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((prefab->components[subGameModel]->gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(9));
		FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, prefab->components[subGameModel]->gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0, int(instanceCount)));

		FE_GL_ERROR(glBindVertexArray(0));
	}
}

void FEEntityInstanced::renderOnlyBillbords(glm::vec3 cameraPosition)
{
	for (size_t i = 0; i < renderers.size(); i++)
	{
		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			if (prefab->components[i]->gameModel->isLODBillboard(j) && prefab->components[i]->gameModel->getLODMesh(j) != nullptr)
			{
				if (renderers[i]->LODBuffers[j] == 0)
					break;

				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, renderers[i]->LODBuffers[j]));

				FE_GL_ERROR(glBindVertexArray(prefab->components[i]->gameModel->getLODMesh(j)->getVaoID()));

				if ((prefab->components[i]->gameModel->getLODMesh(j)->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
				if ((prefab->components[i]->gameModel->getLODMesh(j)->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
				if ((prefab->components[i]->gameModel->getLODMesh(j)->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
				if ((prefab->components[i]->gameModel->getLODMesh(j)->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
				if ((prefab->components[i]->gameModel->getLODMesh(j)->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
				if ((prefab->components[i]->gameModel->getLODMesh(j)->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

				FE_GL_ERROR(glEnableVertexAttribArray(6));
				FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0));
				FE_GL_ERROR(glEnableVertexAttribArray(7));
				FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
				FE_GL_ERROR(glEnableVertexAttribArray(8));
				FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
				FE_GL_ERROR(glEnableVertexAttribArray(9));
				FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

				FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderers[i]->indirectDrawInfoBuffer));
				FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(j * sizeof(FEDrawElementsIndirectCommand))));

				FE_GL_ERROR(glBindVertexArray(0));
				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

				break;
			}
		}
	}
}

void FEEntityInstanced::updateBuffers()
{
	for (size_t i = 0; i < prefab->components.size(); i++)
	{
		if (renderers[i]->instancedBuffer != 0)
		{
			glDeleteBuffers(1, &renderers[i]->instancedBuffer);
		}

		FE_GL_ERROR(glGenBuffers(1, &renderers[i]->instancedBuffer));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, renderers[i]->instancedBuffer));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), renderers[i]->instancedMatrices.data(), GL_DYNAMIC_DRAW));

		unsigned int VAO = prefab->components[i]->gameModel->mesh->getVaoID();
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

		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			if (prefab->components[i]->gameModel->getLODMesh(j) != nullptr)
			{
				unsigned int VAO = prefab->components[i]->gameModel->getLODMesh(j)->getVaoID();
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

		renderers[i]->allInstancesAABB = FEAABB();
		for (size_t j = 0; j < instanceCount; j++)
		{
			glm::mat4 matWithoutTranslate = renderers[i]->transformedInstancedMatrices[j];
			matWithoutTranslate[3][0] -= transform.position.x;
			matWithoutTranslate[3][1] -= transform.position.y;
			matWithoutTranslate[3][2] -= transform.position.z;

			renderers[i]->allInstancesAABB = renderers[i]->allInstancesAABB.merge(prefab->components[i]->gameModel->mesh->AABB.transform(matWithoutTranslate));
		}
	}

	transform.dirtyFlag = true;
	getAABB();
}

void FEEntityInstanced::clear()
{
	instanceCount = 0;

	for (size_t i = 0; i < renderers.size(); i++)
	{
		delete[] renderers[i]->LODCounts;

		renderers[i]->instancedAABBSizes.resize(0);
		renderers[i]->instancedMatrices.resize(0);
		renderers[i]->transformedInstancedMatrices.resize(0);
		renderers[i]->instancePositions.resize(0);
		
		renderers[i]->LODCounts = new int[prefab->components[i]->gameModel->getMaxLODCount()];
		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			renderers[i]->LODCounts[j] = 0;
		}

		renderers[i]->instancedMatricesLOD.resize(prefab->components[i]->gameModel->getMaxLODCount());
	}
	
	transform.setScale(glm::vec3(1.0f));
	modifications.clear();
}

void FEEntityInstanced::addInstanceInternal(glm::mat4 instanceMatrix)
{
	for (size_t i = 0; i < prefab->components.size(); i++)
	{
		renderers[i]->instancedAABBSizes.push_back(-FEAABB(prefab->components[i]->gameModel->getMesh()->getAABB(), instanceMatrix).size);
		renderers[i]->instancedMatrices.push_back(instanceMatrix);
		renderers[i]->transformedInstancedMatrices.push_back(transform.transformMatrix * instanceMatrix);
		renderers[i]->instancePositions.push_back(renderers[i]->transformedInstancedMatrices.back()[3]);

		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			renderers[i]->instancedMatricesLOD[j].resize(instanceCount);
		}
	}

	instanceCount++;
	setDirtyFlag(true);
}

void FEEntityInstanced::addInstances(glm::mat4* instanceMatrix, size_t count)
{
	for (size_t i = 0; i < prefab->components.size(); i++)
	{
		size_t startIndex = renderers[i]->instancedAABBSizes.size();

		renderers[i]->instancedAABBSizes.resize(renderers[i]->instancedAABBSizes.size() + count);
		FEAABB originalAABB = prefab->components[i]->gameModel->getMesh()->getAABB();
		renderers[i]->instancedMatrices.resize(renderers[i]->instancedMatrices.size() + count);
		renderers[i]->transformedInstancedMatrices.resize(renderers[i]->transformedInstancedMatrices.size() + count);
		renderers[i]->instancePositions.resize(renderers[i]->instancePositions.size() + count);

		for (size_t j = startIndex; j < count; j++)
		{
			renderers[i]->instancedAABBSizes[j] = -FEAABB(originalAABB, instanceMatrix[j]).size;
			renderers[i]->instancedMatrices[j] = instanceMatrix[j];
			renderers[i]->transformedInstancedMatrices[j] = transform.transformMatrix * instanceMatrix[j];

			renderers[i]->instancePositions[j] = renderers[i]->transformedInstancedMatrices[j][3];
			if (i == 0)
				instanceCount++;
		}

		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			renderers[i]->instancedMatricesLOD[j].resize(instanceCount);
		}
	}

	setDirtyFlag(true);
}

FEAABB FEEntityInstanced::getAABB()
{
	if (transform.dirtyFlag)
	{
		if (prefab != nullptr)
		{
			if (renderers.size() > 0)
			{
				entityAABB = renderers[0]->allInstancesAABB.transform(transform.getTransformMatrix());
				for (size_t i = 1; i < renderers.size(); i++)
				{
					entityAABB = entityAABB.merge(renderers[i]->allInstancesAABB.transform(transform.getTransformMatrix()));
				}
			}
			else
			{
				entityAABB = FEAABB();
			}
		}

		updateMatrices();
		transform.dirtyFlag = false;
	}

	return entityAABB;
}

int FEEntityInstanced::getInstanceCount()
{
	return int(instanceCount);
}

void FEEntityInstanced::updateMatrices()
{
	for (size_t i = 0; i < renderers.size(); i++)
	{
		if (renderers[i]->instancedMatrices.size() != renderers[i]->transformedInstancedMatrices.size())
		{
			LOG.add("instancedMatrices size and transformedInstancedMatrices size is not equal!", FE_LOG_ERROR, FE_LOG_RENDERING);
			return;
		}

		for (size_t j = 0; j < renderers[i]->instancedMatrices.size(); j++)
		{
			renderers[i]->transformedInstancedMatrices[j] = transform.transformMatrix * renderers[i]->instancedMatrices[j];
			renderers[i]->instancePositions[j] = renderers[i]->transformedInstancedMatrices[j][3];
		}

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderers[i]->sourceDataBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(glm::mat4), renderers[i]->transformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, renderers[i]->positionsBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(float) * 3, renderers[i]->instancePositions.data(), GL_DYNAMIC_DRAW));
	}

	//initializeGPUCulling();
}

bool FEEntityInstanced::populate(FESpawnInfo spawnInfo)
{
	if (spawnInfo.radius <= 0.0f || spawnInfo.objectCount < 1 || spawnInfo.objectCount > 1000000 || prefab == nullptr)
		return false;

	this->spawnInfo = spawnInfo;
	srand(spawnInfo.seed);

	glm::vec3 min = prefab->getAABB().getMin();
	glm::vec3 max = prefab->getAABB().getMax();

	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	ySize *= prefab->components[0]->gameModel->getScaleFactor();

	std::vector<glm::mat4> newMats;
	newMats.resize(spawnInfo.objectCount);

	for (size_t i = 0; i < newMats.size(); i++)
	{
		glm::mat4 newMat = glm::mat4(1.0);
		// spawner transformation would be taken in account later so consider center in 0
		float x = spawnInfo.getPositionDeviation();
		float z = spawnInfo.getPositionDeviation();
		float y = spawnInfo.getPositionDeviation();

		if (renderers[0]->terrainToSnap != nullptr)
		{
			y = std::invoke(renderers[0]->getTerrainY, renderers[0]->terrainToSnap, glm::vec2(transform.position.x + x, transform.position.z + z));

			int countOfTries = 0;
			while (y == -FLT_MAX)
			{
				x = spawnInfo.getPositionDeviation();
				z = spawnInfo.getPositionDeviation();
				y = std::invoke(renderers[0]->getTerrainY, renderers[0]->terrainToSnap, glm::vec2(transform.position.x + x, transform.position.z + z));
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

		float finalScale = prefab->components[0]->gameModel->getScaleFactor() + prefab->components[0]->gameModel->getScaleFactor() * spawnInfo.getScaleDeviation();
		if (finalScale < 0.0f)
			finalScale = 0.01f;
		newMat = glm::scale(newMat, glm::vec3(finalScale));

		newMats[i] = newMat;
	}
	addInstances(newMats.data(), newMats.size());

	if (renderers[0]->terrainToSnap != nullptr)
	{
		// terrain.y could be not 0.0f but here we should indicate 0.0f as Y.
		transform.setPosition(glm::vec3(transform.position.x, 0.0f, transform.position.z));
	}

	srand(unsigned int(time(NULL)));

	setDirtyFlag(true);
	return true;
}

FETerrain* FEEntityInstanced::getSnappedToTerrain()
{
	return renderers[0]->terrainToSnap;
}

void FEEntityInstanced::updateSelectModeAABBData()
{
	instancedAABB.clear();
	instancedAABB.resize(instanceCount);

	for (size_t i = 0; i < instanceCount; i++)
	{
		instancedAABB[i] = FEAABB(prefab->getAABB(), renderers[0]->transformedInstancedMatrices[i]);
	}

	setDirtyFlag(true);
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
	if (instanceIndex < 0 || instanceIndex >= renderers[0]->instancedMatrices.size())
		return;

	modifications.push_back(FEInstanceModification(CHANGE_DELETED, int(instanceIndex), glm::mat4()));

	instanceCount--;
	for (size_t i = 0; i < renderers.size(); i++)
	{
		renderers[i]->instancedAABBSizes.erase(renderers[i]->instancedAABBSizes.begin() + instanceIndex);
		renderers[i]->instancedMatrices.erase(renderers[i]->instancedMatrices.begin() + instanceIndex);
		renderers[i]->transformedInstancedMatrices.erase(renderers[i]->transformedInstancedMatrices.begin() + instanceIndex);
		renderers[i]->instancePositions.erase(renderers[i]->instancePositions.begin() + instanceIndex);

		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			renderers[i]->instancedMatricesLOD[j].resize(instanceCount);
		}
	}

	if (instancedAABB.size() == 0)
	{
		updateSelectModeAABBData();
	}
	else
	{
		instancedAABB.erase(instancedAABB.begin() + instanceIndex);
	}
	
	setDirtyFlag(true);
}

glm::mat4 FEEntityInstanced::getTransformedInstancedMatrix(size_t instanceIndex)
{
	if (instanceIndex < 0 || instanceIndex >= renderers[0]->transformedInstancedMatrices.size())
		return glm::identity<glm::mat4>();

	return renderers[0]->transformedInstancedMatrices[instanceIndex];
}

void FEEntityInstanced::modifyInstance(size_t instanceIndex, glm::mat4 newMatrix)
{
	if (instanceIndex < 0 || instanceIndex >= renderers[0]->transformedInstancedMatrices.size())
		return;

	if (glm::all(glm::epsilonEqual(renderers[0]->transformedInstancedMatrices[instanceIndex][0], newMatrix[0], 0.001f)) &&
		glm::all(glm::epsilonEqual(renderers[0]->transformedInstancedMatrices[instanceIndex][1], newMatrix[1], 0.001f)) &&
		glm::all(glm::epsilonEqual(renderers[0]->transformedInstancedMatrices[instanceIndex][2], newMatrix[2], 0.001f)) &&
		glm::all(glm::epsilonEqual(renderers[0]->transformedInstancedMatrices[instanceIndex][3], newMatrix[3], 0.001f)))
		return;

	if (modifications.size() > 0 && modifications.back().index == instanceIndex && modifications.back().type == CHANGE_MODIFIED)
	{
		modifications.back().modification = newMatrix;
	}
	else
	{
		modifications.push_back(FEInstanceModification(CHANGE_MODIFIED, int(instanceIndex), newMatrix));
	}
	
	for (size_t i = 0; i < renderers.size(); i++)
	{
		renderers[i]->transformedInstancedMatrices[instanceIndex] = newMatrix;
		renderers[i]->instancedMatrices[instanceIndex] = glm::inverse(transform.getTransformMatrix()) * newMatrix;

		if (instancedAABB.size() > instanceIndex)
			instancedAABB[instanceIndex] = FEAABB(prefab->getAABB(), newMatrix);
		renderers[i]->instancedAABBSizes[instanceIndex] = -FEAABB(prefab->getAABB(), newMatrix).size;
	}

	setDirtyFlag(true);
}

int FEEntityInstanced::getSpawnModificationCount()
{
	return int(modifications.size());
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
		instancedAABB.push_back(FEAABB(prefab->getAABB(), renderers[0]->transformedInstancedMatrices.back()));
	}

	modifications.push_back(FEInstanceModification(CHANGE_ADDED, int(instanceCount), instanceMatrix));
	setDirtyFlag(true);
}

bool FEEntityInstanced::tryToSnapInstance(size_t instanceIndex)
{
	if (instanceIndex < 0 || instanceIndex >= renderers[0]->transformedInstancedMatrices.size() || renderers[0]->terrainToSnap == nullptr)
		return false;

	if (!isSelectMode())
		return false;

	float y = std::invoke(renderers[0]->getTerrainY, renderers[0]->terrainToSnap, glm::vec2(renderers[0]->transformedInstancedMatrices[instanceIndex][3][0], renderers[0]->transformedInstancedMatrices[instanceIndex][3][2]));
	if (y == -FLT_MAX)
		return false;

	if (abs(renderers[0]->transformedInstancedMatrices[instanceIndex][3][1] - y) < 0.01f)
		return true;

	glm::mat4 copy = renderers[0]->transformedInstancedMatrices[instanceIndex];
	copy[3][1] = y;
	modifyInstance(instanceIndex, copy);
	setDirtyFlag(true);
	return true;
}

void FEEntityInstanced::initializeGPUCulling()
{
	for (size_t i = 0; i < prefab->components.size(); i++)
	{
		if (renderers[i]->sourceDataBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->sourceDataBuffer));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->sourceDataBuffer));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderers[i]->sourceDataBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(glm::mat4), renderers[i]->transformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

		if (renderers[i]->positionsBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->positionsBuffer));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->positionsBuffer));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, renderers[i]->positionsBuffer));

		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(float) * 3, renderers[i]->instancePositions.data(), GL_DYNAMIC_DRAW));

		if (renderers[i]->LODBuffers[0] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->LODBuffers[0]));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->LODBuffers[0]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, renderers[i]->LODBuffers[0]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (renderers[i]->LODBuffers[1] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->LODBuffers[1]));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->LODBuffers[1]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, renderers[i]->LODBuffers[1]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (renderers[i]->LODBuffers[2] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->LODBuffers[2]));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->LODBuffers[2]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, renderers[i]->LODBuffers[2]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (renderers[i]->LODBuffers[3] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->LODBuffers[3]));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->LODBuffers[3]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, renderers[i]->LODBuffers[3]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (renderers[i]->AABBSizesBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->AABBSizesBuffer));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->AABBSizesBuffer));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, renderers[i]->AABBSizesBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, instanceCount * sizeof(float), renderers[i]->instancedAABBSizes.data(), GL_DYNAMIC_DRAW));

		std::vector<float> LODInfoData;
		LODInfoData.push_back(prefab->components[i]->gameModel->getCullDistance());
		LODInfoData.push_back(prefab->components[i]->gameModel->getLODMaxDrawDistance(0));
		LODInfoData.push_back(prefab->components[i]->gameModel->getLODMaxDrawDistance(1));
		LODInfoData.push_back(prefab->components[i]->gameModel->getLODMaxDrawDistance(2));

		// does it have billboard ?
		unsigned int billboardIndex = 5;
		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			if (prefab->components[i]->gameModel->isLODBillboard(j) && prefab->components[i]->gameModel->getLODMesh(j) != nullptr)
			{
				billboardIndex = int(j);
			}
		}

		LODInfoData.push_back(float(billboardIndex));
		// this should not be here, instead normal of plane should align with vector to camera
		LODInfoData.push_back(1.5708f * 3.0f + prefab->components[i]->gameModel->getBillboardZeroRotaion() * ANGLE_TORADIANS_COF);
		LODInfoData.push_back(float(instanceCount));

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, renderers[i]->LODInfoBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, 7 * sizeof(float), LODInfoData.data(), GL_DYNAMIC_DRAW));

		if (renderers[i]->indirectDrawsInfo == nullptr)
			renderers[i]->indirectDrawsInfo = new FEDrawElementsIndirectCommand[4];
		for (size_t j = 0; j < prefab->components[i]->gameModel->getMaxLODCount(); j++)
		{
			renderers[i]->indirectDrawsInfo[j].count = prefab->components[i]->gameModel->getLODMesh(j) == nullptr ? 0 : prefab->components[i]->gameModel->getLODMesh(j)->getVertexCount();
			renderers[i]->indirectDrawsInfo[j].baseInstance = 0;
			renderers[i]->indirectDrawsInfo[j].baseVertex = 0;
			renderers[i]->indirectDrawsInfo[j].firstIndex = 0;
			renderers[i]->indirectDrawsInfo[j].primCount = 0;
		}

		if (renderers[i]->indirectDrawInfoBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &renderers[i]->indirectDrawInfoBuffer));
			FE_GL_ERROR(glGenBuffers(1, &renderers[i]->indirectDrawInfoBuffer));
		}

		FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, renderers[i]->indirectDrawInfoBuffer));
		FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, renderers[i]->indirectDrawsInfo, GL_MAP_READ_BIT));
		FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}
}

void FEEntityInstanced::snapToTerrain(FETerrain* terrain, float(FETerrain::* getTerrainY)(glm::vec2))
{
	for (size_t i = 0; i < prefab->components.size(); i++)
	{
		renderers[i]->terrainToSnap = terrain;
		renderers[i]->getTerrainY = getTerrainY;
	}
}

void FEEntityInstanced::unSnapFromTerrain()
{
	for (size_t i = 0; i < prefab->components.size(); i++)
	{
		renderers[i]->terrainToSnap = nullptr;
	}
}

void FEEntityInstanced::clearRenderers()
{
	if (prefab->components.size() > renderers.size())
	{
		int countBefore = int(renderers.size());
		renderers.resize(prefab->components.size());
		for (int i = countBefore; i < renderers.size(); i++)
		{
			initRender(i);

			renderers[i]->instancedMatrices = renderers[0]->instancedMatrices;
			renderers[i]->transformedInstancedMatrices = renderers[0]->transformedInstancedMatrices;
			renderers[i]->instancePositions = renderers[0]->instancePositions;
		}
	}
	else if (prefab->components.size() < renderers.size())
	{
		renderers.erase(renderers.begin() + renderers.size() - 1, renderers.end());
	}
}

void FEEntityInstanced::checkDirtyFlag(int subGameModel)
{
	if (renderers[subGameModel]->lastFramePrefab != prefab || prefab->getDirtyFlag())
	{
		setDirtyFlag(true);
		renderers[subGameModel]->lastFramePrefab = prefab;
	}

	for (int i = 0; i < prefab->componentsCount(); i++)
	{
		if (prefab->getComponent(i)->gameModel->getDirtyFlag() || prefab->getComponent(i)->transform.getDirtyFlag())
		{
			setDirtyFlag(true);
			break;
		}
	}

	if (getDirtyFlag())
	{
		clearRenderers();
		updateBuffers();
		setDirtyFlag(false);
		initializeGPUCulling();
		//prefab->components[subGameModel]->gameModel->dirtyFlag = false;
		prefab->setDirtyFlag(false);
		for (int i = 0; i < prefab->componentsCount(); i++)
		{
			prefab->getComponent(i)->gameModel->dirtyFlag = false;
			prefab->getComponent(i)->transform.setDirtyFlag(false);
		}
	}

	if (transform.dirtyFlag)
	{
		updateMatrices();
	}
}