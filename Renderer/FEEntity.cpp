#include "FEEntity.h"

FocalEngine::FEEntity::FEEntity(FEMesh* Mesh, FEMaterial* Material)
{
	mesh = Mesh;
	material = Material;
}

FocalEngine::FEEntity::~FEEntity()
{
	delete mesh;
	delete material;
}

void FocalEngine::FEEntity::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
}

void FocalEngine::FEEntity::render()
{
	//material->bind();

	glBindVertexArray(mesh->getVaoID());
	glEnableVertexAttribArray(0);

	//glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount());

	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	//material->unBind();
}