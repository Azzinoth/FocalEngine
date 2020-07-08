#include "FEEntity.h"
using namespace FocalEngine;

FEEntity::FEEntity(FEGameModel* gameModel, std::string Name)
{
	this->gameModel = gameModel;
	name = Name;
	nameHash = std::hash<std::string>{}(name);
}

FEEntity::~FEEntity()
{
}

void FEEntity::render()
{
	FE_GL_ERROR(glBindVertexArray(gameModel->mesh->getVaoID()));
	if ((gameModel->mesh->vertexBuffers & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((gameModel->mesh->vertexBuffers & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((gameModel->mesh->vertexBuffers & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((gameModel->mesh->vertexBuffers & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((gameModel->mesh->vertexBuffers & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((gameModel->mesh->vertexBuffers & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((gameModel->mesh->vertexBuffers & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, gameModel->mesh->getVertexCount()));

	if ((gameModel->mesh->vertexBuffers & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glDisableVertexAttribArray(0));
	if ((gameModel->mesh->vertexBuffers & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glDisableVertexAttribArray(1));
	if ((gameModel->mesh->vertexBuffers & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glDisableVertexAttribArray(2));
	if ((gameModel->mesh->vertexBuffers & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glDisableVertexAttribArray(3));
	if ((gameModel->mesh->vertexBuffers & FE_UV) == FE_UV) FE_GL_ERROR(glDisableVertexAttribArray(4));
	FE_GL_ERROR(glBindVertexArray(0));
}

bool FEEntity::isVisible()
{
	return visible;
}

void FEEntity::setVisibility(bool isVisible)
{
	visible = isVisible;
}

std::string FEEntity::getName()
{
	return name;
}

void FEEntity::setName(std::string newName)
{
	name = newName;
	nameHash = std::hash<std::string>{}(name);
}

FEAABB FEEntity::getAABB()
{
	//#fix it should be optimized.
	FEAABB& meshAABB = gameModel->mesh->AABB;
	// firstly we generate 8 points that represent AABCube.
	// bottom 4 points
	glm::vec4 bottomLeftFront = glm::vec4(meshAABB.min.x, meshAABB.min.y, meshAABB.max.z, 1.0f);
	glm::vec4 bottomRightFront = glm::vec4(meshAABB.max.x, meshAABB.min.y, meshAABB.max.z, 1.0f);
	glm::vec4 bottomRightBack = glm::vec4(meshAABB.max.x, meshAABB.min.y, meshAABB.min.z, 1.0f);
	glm::vec4 bottomLeftBack = glm::vec4(meshAABB.min.x, meshAABB.min.y, meshAABB.min.z, 1.0f);
	// top 4 points
	glm::vec4 topLeftFront = glm::vec4(meshAABB.min.x, meshAABB.max.y, meshAABB.max.z, 1.0f);
	glm::vec4 topRightFront = glm::vec4(meshAABB.max.x, meshAABB.max.y, meshAABB.max.z, 1.0f);
	glm::vec4 topRightBack = glm::vec4(meshAABB.max.x, meshAABB.max.y, meshAABB.min.z, 1.0f);
	glm::vec4 topLeftBack = glm::vec4(meshAABB.min.x, meshAABB.max.y, meshAABB.min.z, 1.0f);

	// transform each point of this cube
	bottomLeftFront = transform.getTransformMatrix() * bottomLeftFront;
	bottomRightFront = transform.getTransformMatrix() * bottomRightFront;
	bottomRightBack = transform.getTransformMatrix() * bottomRightBack;
	bottomLeftBack = transform.getTransformMatrix() * bottomLeftBack;

	topLeftFront = transform.getTransformMatrix() * topLeftFront;
	topRightFront = transform.getTransformMatrix() * topRightFront;
	topRightBack = transform.getTransformMatrix() * topRightBack;
	topLeftBack = transform.getTransformMatrix() * topLeftBack;

	// for more convenient searching
	std::vector<glm::vec4> allPoints;
	allPoints.push_back(bottomLeftFront);
	allPoints.push_back(bottomRightFront);
	allPoints.push_back(bottomRightBack);
	allPoints.push_back(bottomLeftBack);

	allPoints.push_back(topLeftFront);
	allPoints.push_back(topRightFront);
	allPoints.push_back(topRightBack);
	allPoints.push_back(topLeftBack);

	// identify new AABB
	FEAABB entityAABB;

	entityAABB.min = glm::vec3(FLT_MAX);
	entityAABB.max = glm::vec3(-FLT_MAX);
	for (auto point : allPoints)
	{
		if (point.x < entityAABB.min.x)
			entityAABB.min.x = point.x;

		if (point.x > entityAABB.max.x)
			entityAABB.max.x = point.x;

		if (point.y < entityAABB.min.y)
			entityAABB.min.y = point.y;

		if (point.y > entityAABB.max.y)
			entityAABB.max.y = point.y;

		if (point.z < entityAABB.min.z)
			entityAABB.min.z = point.z;

		if (point.z > entityAABB.max.z)
			entityAABB.max.z = point.z;
	}

	return entityAABB;
}

FEAABB FEEntity::getPureAABB()
{
	return gameModel->mesh->AABB;
}

bool FEEntity::isCastShadows()
{
	return castShadows;
}

void FEEntity::setCastShadows(bool isCastShadows)
{
	castShadows = isCastShadows;
}

bool FEEntity::isReceivingShadows()
{
	return receiveShadows;
}

void FEEntity::setReceivingShadows(bool isReceivingShadows)
{
	receiveShadows = isReceivingShadows;
}

int FEEntity::getNameHash()
{
	return nameHash;
}