#include "FETerrain.h"
using namespace FocalEngine;

FETerrain::FETerrain(std::string Name)
{
	name = Name;
	nameHash = std::hash<std::string>{}(name);
}

FETerrain::~FETerrain()
{
}

void FETerrain::render()
{
	//static glm::vec3 pivotPosition = transform.getPosition();
	//pivotPosition = transform.getPosition();

	//for (size_t i = 0; i < 4; i++)
	//{
	//	for (size_t j = 0; j < 4; j++)
	//	{
	//		transform.setPosition(glm::vec3(pivotPosition.x + i * 64.0f, pivotPosition.y, pivotPosition.z + j * 64.0f));
	//		shader->getParameter("FEWorldMatrix")->updateData(transform.getTransformMatrix());

	//		//#fix that
	//		shader->loadDataToGPU();
	//		//shader->getParameter("hightMapShift")->updateData(glm::vec2(i * 64.0, ));
			FE_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64));
		/*}
	}
	
	transform.setPosition(pivotPosition);*/
}

bool FETerrain::isVisible()
{
	return visible;
}

void FETerrain::setVisibility(bool isVisible)
{
	visible = isVisible;
}

std::string FETerrain::getName()
{
	return name;
}

void FETerrain::setName(std::string newName)
{
	name = newName;
	nameHash = std::hash<std::string>{}(name);
}

//FEAABB FETerrain::getAABB()
//{
	////#fix it should be optimized.
	//FEAABB& meshAABB = gameModel->mesh->AABB;
	//// firstly we generate 8 points that represent AABCube.
	//// bottom 4 points
	//glm::vec4 bottomLeftFront = glm::vec4(meshAABB.min.x, meshAABB.min.y, meshAABB.max.z, 1.0f);
	//glm::vec4 bottomRightFront = glm::vec4(meshAABB.max.x, meshAABB.min.y, meshAABB.max.z, 1.0f);
	//glm::vec4 bottomRightBack = glm::vec4(meshAABB.max.x, meshAABB.min.y, meshAABB.min.z, 1.0f);
	//glm::vec4 bottomLeftBack = glm::vec4(meshAABB.min.x, meshAABB.min.y, meshAABB.min.z, 1.0f);
	//// top 4 points
	//glm::vec4 topLeftFront = glm::vec4(meshAABB.min.x, meshAABB.max.y, meshAABB.max.z, 1.0f);
	//glm::vec4 topRightFront = glm::vec4(meshAABB.max.x, meshAABB.max.y, meshAABB.max.z, 1.0f);
	//glm::vec4 topRightBack = glm::vec4(meshAABB.max.x, meshAABB.max.y, meshAABB.min.z, 1.0f);
	//glm::vec4 topLeftBack = glm::vec4(meshAABB.min.x, meshAABB.max.y, meshAABB.min.z, 1.0f);

	//// transform each point of this cube
	//bottomLeftFront = transform.getTransformMatrix() * bottomLeftFront;
	//bottomRightFront = transform.getTransformMatrix() * bottomRightFront;
	//bottomRightBack = transform.getTransformMatrix() * bottomRightBack;
	//bottomLeftBack = transform.getTransformMatrix() * bottomLeftBack;

	//topLeftFront = transform.getTransformMatrix() * topLeftFront;
	//topRightFront = transform.getTransformMatrix() * topRightFront;
	//topRightBack = transform.getTransformMatrix() * topRightBack;
	//topLeftBack = transform.getTransformMatrix() * topLeftBack;

	//// for more convenient searching
	//std::vector<glm::vec4> allPoints;
	//allPoints.push_back(bottomLeftFront);
	//allPoints.push_back(bottomRightFront);
	//allPoints.push_back(bottomRightBack);
	//allPoints.push_back(bottomLeftBack);

	//allPoints.push_back(topLeftFront);
	//allPoints.push_back(topRightFront);
	//allPoints.push_back(topRightBack);
	//allPoints.push_back(topLeftBack);

	//// identify new AABB
	//FEAABB entityAABB;

	//entityAABB.min = glm::vec3(FLT_MAX);
	//entityAABB.max = glm::vec3(-FLT_MAX);
	//for (auto point : allPoints)
	//{
	//	if (point.x < entityAABB.min.x)
	//		entityAABB.min.x = point.x;

	//	if (point.x > entityAABB.max.x)
	//		entityAABB.max.x = point.x;

	//	if (point.y < entityAABB.min.y)
	//		entityAABB.min.y = point.y;

	//	if (point.y > entityAABB.max.y)
	//		entityAABB.max.y = point.y;

	//	if (point.z < entityAABB.min.z)
	//		entityAABB.min.z = point.z;

	//	if (point.z > entityAABB.max.z)
	//		entityAABB.max.z = point.z;
	//}

	//return entityAABB;
//}

//FEAABB FETerrain::getPureAABB()
//{
//	return gameModel->mesh->AABB;
//}

bool FETerrain::isCastShadows()
{
	return castShadows;
}

void FETerrain::setCastShadows(bool isCastShadows)
{
	castShadows = isCastShadows;
}

bool FETerrain::isReceivingShadows()
{
	return receiveShadows;
}

void FETerrain::setReceivingShadows(bool isReceivingShadows)
{
	receiveShadows = isReceivingShadows;
}

int FETerrain::getNameHash()
{
	return nameHash;
}

void FETerrain::setWireframeMode(bool isActive)
{
	wireframeMode = isActive;
}

bool FETerrain::isWireframeMode()
{
	return wireframeMode;
}

float FETerrain::getHightScale()
{
	return hightScale;
}

void FETerrain::setHightScale(float newHightScale)
{
	hightScale = newHightScale;
}

glm::vec2 FETerrain::getTileMult()
{
	return tileMult;
}

void FETerrain::setTileMult(glm::vec2 newTileMult)
{
	tileMult = newTileMult;
}

float FETerrain::getLODlevel()
{
	return LODlevel;
}

void FETerrain::setLODlevel(float newLODlevel)
{
	if (newLODlevel < 2.0)
		newLODlevel = 2.0;

	if (newLODlevel > 128.0)
		newLODlevel = 128.0;

	LODlevel = newLODlevel;
}

float FETerrain::getChunkPerSide()
{
	return chunkPerSide;
}

void FETerrain::setChunkPerSide(float newChunkPerSide)
{
	if (newChunkPerSide < 1.0f)
		newChunkPerSide = 1.0f;

	if (newChunkPerSide > 16.0f)
		newChunkPerSide = 16.0f;

	chunkPerSide = newChunkPerSide;
}

float FETerrain::getDisplacementScale()
{
	return displacementScale;
}

void FETerrain::setDisplacementScale(float newDisplacementScale)
{
	displacementScale = newDisplacementScale;
}
