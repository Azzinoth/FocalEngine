#include "FEEntity.h"
using namespace FocalEngine;

FEEntity::FEEntity(FEMesh* Mesh, FEMaterial* Material)
{
	mesh = Mesh;
	material = Material;
}

FEEntity::FEEntity(FEMesh* Mesh)
{
	mesh = Mesh;
	material = new FEStandardMaterial();
}

FEEntity::~FEEntity()
{
	delete mesh;
	delete material;
}

glm::vec3 FEEntity::getPosition()
{
	return position;
}

glm::vec3 FEEntity::getRotation()
{
	return rotation;
}

glm::vec3 FEEntity::getScale()
{
	return scale;
}

void FEEntity::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	updateWorldMatrix();
}

void FEEntity::setRotation(glm::vec3 newRotation)
{
	rotation = newRotation;
	updateWorldMatrix();
}

void FEEntity::setScale(glm::vec3 newScale)
{
	scale = newScale;
	updateWorldMatrix();
}

void FEEntity::updateWorldMatrix()
{
	worldMatrix = glm::mat4(1.0);
	worldMatrix = glm::translate(worldMatrix, position);

	worldMatrix = glm::rotate(worldMatrix, (float)rotation.x * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	worldMatrix = glm::rotate(worldMatrix, (float)rotation.y * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	worldMatrix = glm::rotate(worldMatrix, (float)rotation.z * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

	worldMatrix = glm::scale(worldMatrix, glm::vec3(scale[0], scale[1], scale[2]));
}

void FEEntity::render()
{
	glBindVertexArray(mesh->getVaoID());
	if ((mesh->vertexBuffers & FE_POSITION) == FE_POSITION) glEnableVertexAttribArray(0);
	if ((mesh->vertexBuffers & FE_COLOR) == FE_COLOR) glEnableVertexAttribArray(1);
	if ((mesh->vertexBuffers & FE_NORMAL) == FE_NORMAL) glEnableVertexAttribArray(2);
	if ((mesh->vertexBuffers & FE_TANGENTS) == FE_TANGENTS) glEnableVertexAttribArray(3);
	if ((mesh->vertexBuffers & FE_UV) == FE_UV) glEnableVertexAttribArray(4);

	//glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());

	if ((mesh->vertexBuffers & FE_POSITION) == FE_POSITION) glDisableVertexAttribArray(0);
	if ((mesh->vertexBuffers & FE_COLOR) == FE_COLOR) glDisableVertexAttribArray(1);
	if ((mesh->vertexBuffers & FE_NORMAL) == FE_NORMAL) glDisableVertexAttribArray(2);
	if ((mesh->vertexBuffers & FE_TANGENTS) == FE_TANGENTS) glDisableVertexAttribArray(3);
	if ((mesh->vertexBuffers & FE_UV) == FE_UV) glDisableVertexAttribArray(4);
	glBindVertexArray(0);
}