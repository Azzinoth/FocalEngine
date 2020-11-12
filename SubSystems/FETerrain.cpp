#include "FETerrain.h"
using namespace FocalEngine;

FETerrain::FETerrain(std::string Name) : FEAsset(FE_TERRAIN, Name)
{
	name = Name;
	nameHash = std::hash<std::string>{}(name);
}

FETerrain::~FETerrain()
{
}

void FETerrain::render()
{
	FE_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64));
}

bool FETerrain::isVisible()
{
	return visible;
}

void FETerrain::setVisibility(bool isVisible)
{
	visible = isVisible;
}

FEAABB FETerrain::getAABB()
{
	if (transform.dirtyFlag)
	{
		transform.dirtyFlag = false;
		//#fix it should be optimized.
		FEAABB meshAABB = AABB;
		// -0.5f it is a little hack, because this -0.5f should be made during tessellation.
		meshAABB.min = glm::vec3(-32.0f - 0.5f, AABB.getMin()[1], -32.0f - 0.5f);
		meshAABB.max = glm::vec3(32.0f + 64.0f * (chunkPerSide - 1) - 0.5f, AABB.getMax()[1], 32.0f + 64.0f * (chunkPerSide - 1) - 0.5f);
		meshAABB = FEAABB(glm::vec3(meshAABB.getMin()[0], meshAABB.getMin()[1] * hightScale, meshAABB.getMin()[2]), glm::vec3(meshAABB.getMax()[0], meshAABB.getMax()[1] * hightScale, meshAABB.getMax()[2]));

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

		glm::mat4 transformMatrix = transform.getTransformMatrix();
		// transform each point of this cube
		bottomLeftFront = transformMatrix * bottomLeftFront;
		bottomRightFront = transformMatrix * bottomRightFront;
		bottomRightBack = transformMatrix * bottomRightBack;
		bottomLeftBack = transformMatrix * bottomLeftBack;

		topLeftFront = transformMatrix * topLeftFront;
		topRightFront = transformMatrix * topRightFront;
		topRightBack = transformMatrix * topRightBack;
		topLeftBack = transformMatrix * topLeftBack;

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
		finalAABB.min = glm::vec3(FLT_MAX);
		finalAABB.max = glm::vec3(-FLT_MAX);
		for (auto point : allPoints)
		{
			if (point.x < finalAABB.min.x)
				finalAABB.min.x = point.x;

			if (point.x > finalAABB.max.x)
				finalAABB.max.x = point.x;

			if (point.y < finalAABB.min.y)
				finalAABB.min.y = point.y;

			if (point.y > finalAABB.max.y)
				finalAABB.max.y = point.y;

			if (point.z < finalAABB.min.z)
				finalAABB.min.z = point.z;

			if (point.z > finalAABB.max.z)
				finalAABB.max.z = point.z;
		}

		xSize = finalAABB.getMax()[0] - finalAABB.getMin()[0];
		zSize = finalAABB.getMax()[2] - finalAABB.getMin()[2];
	}

	return finalAABB;
}

FEAABB FETerrain::getPureAABB()
{
	FEAABB meshAABB = AABB;
	meshAABB.min = glm::vec3(-32.0f, AABB.getMin()[1], -32.0f);
	meshAABB.max = glm::vec3(32.0f + 64.0f * (chunkPerSide - 1), AABB.getMax()[1], 32.0f + 64.0f * (chunkPerSide - 1));
	meshAABB = FEAABB(glm::vec3(meshAABB.getMin()[0], meshAABB.getMin()[1] * hightScale, meshAABB.getMin()[2]), glm::vec3(meshAABB.getMax()[0], meshAABB.getMax()[1] * hightScale, meshAABB.getMax()[2]));
	return meshAABB;
}

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
	if (hightScale != newHightScale)
		transform.dirtyFlag = true;
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

	if (chunkPerSide != newChunkPerSide)
		transform.dirtyFlag = true;
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

float FETerrain::getHeightAt(glm::vec2 XZWorldPosition)
{
	float localX = XZWorldPosition[0];
	float localZ = XZWorldPosition[1];

	localX -= finalAABB.getMin()[0];
	localZ -= finalAABB.getMin()[2];

	if (xSize == 0 || zSize == 0)
		getAABB();

	localX = localX / xSize;
	localZ = localZ / zSize;

	if (localX > 0 && localZ > 0 && localX < 1.0 && localZ < 1.0)
	{
		localX = float(int(localX * this->heightMap->getWidth()));
		localZ = float(int(localZ * this->heightMap->getHeight()));

		int index = int(localZ * this->heightMap->getWidth() + localX);
		return heightMapArray[index] * hightScale * transform.getScale()[1] + transform.getPosition()[1];
	}

	return -FLT_MAX;
}

float FETerrain::getXSize()
{
	return xSize;
}

float FETerrain::getZSize()
{
	return zSize;
}

// ********************************** PointOnTerrain **********************************
bool FETerrain::isUnderGround(glm::dvec3 testPoint)
{
	float height = getHeightAt(glm::vec2(testPoint.x, testPoint.z));
	// if we go outside terrain.
	if (height == -FLT_MAX)
		return true;

	return testPoint.y < height ? true : false;
}

glm::dvec3 FETerrain::getPointOnRay(glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection, float distance)
{
	glm::dvec3 start = glm::dvec3(mouseRayStart.x, mouseRayStart.y, mouseRayStart.z);
	glm::dvec3 scaledRay = glm::dvec3(mouseRayDirection.x * distance, mouseRayDirection.y * distance, mouseRayDirection.z * distance);

	return start + scaledRay;
}

bool FETerrain::intersectionInRange(float start, float finish, glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection)
{
	glm::dvec3 startPoint = getPointOnRay(mouseRayStart, mouseRayDirection, start);
	glm::dvec3 endPoint = getPointOnRay(mouseRayStart, mouseRayDirection, finish);
	return !isUnderGround(startPoint) && isUnderGround(endPoint) ? true : false;
	/*if (!isUnderGround(startPoint) && isUnderGround(endPoint))
	{
		return true;
	}
	else
	{
		return false;
	}*/
}

glm::dvec3 FETerrain::binarySearch(int count, float start, float finish, glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection)
{
	const int RECURSION_COUNT = 200;
	float half = start + ((finish - start) / 2.0f);
	if (count >= RECURSION_COUNT)
	{
		glm::dvec3 endPoint = getPointOnRay(mouseRayStart, mouseRayDirection, half);
		if (getHeightAt(glm::vec2(endPoint.x, endPoint.z)) != -1.0f)
		{
			return endPoint;
		}
		else
		{
			return glm::dvec3(FLT_MAX);
		}
	}

	if (intersectionInRange(start, half, mouseRayStart, mouseRayDirection))
	{
		return binarySearch(count + 1, start, half, mouseRayStart, mouseRayDirection);
	}
	else
	{
		return binarySearch(count + 1, half, finish, mouseRayStart, mouseRayDirection);
	}
}

glm::dvec3 FETerrain::getPointOnTerrain(glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection, float startDistance, float endDistance)
{
	if (intersectionInRange(startDistance, endDistance, mouseRayStart, mouseRayDirection))
	{
		glm::dvec3 pointOnTerrain = binarySearch(0, startDistance, endDistance, mouseRayStart, mouseRayDirection);
		// if point is not above terrain, point could be above because isUnderGround returning true if we go out of terrain bounds to fix some bugs.
		if ((getHeightAt(glm::vec2(pointOnTerrain.x, pointOnTerrain.z)) + 1.0f) > pointOnTerrain.y)
			return pointOnTerrain;

		return glm::dvec3(FLT_MAX);
	}

	return glm::dvec3(FLT_MAX);
}

void FETerrain::updateCPUHeightInfo()
{
	if (heightMap->getWidth() * heightMap->getHeight() * 2 != pixelBufferCount)
	{
		pixelBufferCount = heightMap->getWidth() * heightMap->getHeight() * 2;
		delete[] pixelBuffer;
		pixelBuffer = new char[pixelBufferCount];
	}

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, heightMap->getTextureID()));
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, pixelBuffer));
	heightMap->unBind();

	float max = FLT_MIN;
	float min = FLT_MAX;
	int iterator = 0;
	for (size_t i = 0; i < pixelBufferCount; i += 2)
	{
		unsigned short temp = *(unsigned short*)(&pixelBuffer[i]);
		heightMapArray[iterator] = temp / float(0xFFFF);

		if (max < heightMapArray[iterator])
			max = heightMapArray[iterator];

		if (min > heightMapArray[iterator])
			min = heightMapArray[iterator];

		iterator++;
	}

	glm::vec3 minPoint = glm::vec3(-1.0f, min, -1.0f);
	glm::vec3 maxPoint = glm::vec3(1.0f, max, 1.0f);
	AABB = FEAABB(minPoint, maxPoint);
	transform.dirtyFlag = true;
}
// ********************************** PointOnTerrain END **********************************

// **************************** TERRAIN EDITOR TOOLS ****************************
bool FETerrain::isBrushActive()
{
	return brushActive;
}

void FETerrain::setBrushActive(bool newBrushActive)
{
	brushActive = newBrushActive;
}

bool FETerrain::isBrushInversed()
{
	return brushInversed;
}

bool FETerrain::isBrushSculptMode()
{
	return brushSculptMode;
}

void FETerrain::setBrushSculptMode(bool newBrushSculptMode)
{
	brushSculptMode = newBrushSculptMode;
}

bool FETerrain::isBrushLevelMode()
{
	return brushLevelMode;
}

void FETerrain::setBrushLevelMode(bool newBrushLevelMode)
{
	brushLevelMode = newBrushLevelMode;
}

bool FETerrain::isBrushSmoothMode()
{
	return brushSmoothMode;
}

void FETerrain::setBrushSmoothMode(bool newBrushSmoothMode)
{
	brushSmoothMode = newBrushSmoothMode;
}

void FETerrain::setBrushInversed(bool newBrushInversed)
{
	brushInversed = newBrushInversed;
}

float FETerrain::getBrushSize()
{
	return brushSize;
}

void FETerrain::setBrushSize(float newBrushSize)
{
	if (newBrushSize <= 0.0f)
		newBrushSize = 0.001f;

	if (newBrushSize > 1000.0f)
		newBrushSize = 1000.0f;

	brushSize = newBrushSize;
}

float FETerrain::getBrushIntensity()
{
	return brushIntensity;
}

void FETerrain::setBrushIntensity(float newBrushIntensity)
{
	if (newBrushIntensity <= 0.0f)
		newBrushIntensity = 0.001f;

	if (newBrushIntensity > 1000.0f)
		newBrushIntensity = 1000.0f;

	brushIntensity = newBrushIntensity;
}

void FETerrain::updateBrush(glm::dvec3 mouseRayStart, glm::dvec3 mouseRayDirection)
{
	if (!brushSculptMode && !brushLevelMode && !brushSmoothMode)
	{
		if (brushVisualFBCleared)
			return;

		// get current clear color.
		GLfloat bkColor[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor);

		brushVisualFB->bind();
		FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));
		brushVisualFB->unBind();
		brushVisualFBCleared = true;

		FE_GL_ERROR(glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]));

		return;
	}

	callCount++;
	if (callCount > framesBeforeUpdate && CPUHeightInfoDirtyFlag)
	{
		callCount = 0;
		CPUHeightInfoDirtyFlag = false;
		updateCPUHeightInfo();
	}

	float range = getXSize() * 2.0f;
	glm::dvec3 currentTerrainPoint = getPointOnTerrain(mouseRayStart, mouseRayDirection, 0, range);

	float localX = float(currentTerrainPoint.x);
	float localZ = float(currentTerrainPoint.z);

	localX -= getAABB().getMin()[0];
	localZ -= getAABB().getMin()[2];

	localX = localX / getXSize();
	localZ = localZ / getZSize();

	if (isBrushActive())
	{
		CPUHeightInfoDirtyFlag = true;
		heightMap->setDirtyFlag(true);

		if (localX > 0 && localZ > 0 && localX < 1.0 && localZ < 1.0)
		{
			brushOutputShader->getParameter("brushCenter")->updateData(glm::vec2(localX, localZ));
		}

		brushOutputShader->getParameter("brushSize")->updateData(brushSize / (getXSize() * 2.0f));

		if (brushSculptMode && !brushInversed)
		{
			brushOutputShader->getParameter("brushMode")->updateData(0.0f);
		}
		else if (brushSculptMode && brushInversed)
		{
			brushOutputShader->getParameter("brushMode")->updateData(1.0f);
		}
		else if (brushLevelMode)
		{
			brushOutputShader->getParameter("brushMode")->updateData(2.0f);
		}
		else if (brushSmoothMode)
		{
			brushOutputShader->getParameter("brushMode")->updateData(3.0f);
		}
		
		brushOutputShader->getParameter("brushIntensity")->updateData(brushIntensity);

		brushOutputFB->bind();
		brushOutputShader->start();

		brushOutputShader->loadDataToGPU();
		heightMap->bind(0);
		FE_GL_ERROR(glViewport(0, 0, heightMap->getWidth(), heightMap->getHeight()));

		FE_GL_ERROR(glBindVertexArray(planeMesh->getVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, planeMesh->getVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		heightMap->unBind();
		brushOutputShader->stop();
		brushOutputFB->unBind();
	}

	if (localX > 0 && localZ > 0 && localX < 1.0 && localZ < 1.0)
	{
		brushVisualShader->getParameter("brushCenter")->updateData(glm::vec2(localX, localZ));
	}
	brushVisualShader->getParameter("brushSize")->updateData(brushSize / (getXSize() * 2.0f));

	brushVisualFB->bind();
	brushVisualShader->start();

	brushVisualShader->loadDataToGPU();
	FE_GL_ERROR(glViewport(0, 0, heightMap->getWidth(), heightMap->getHeight()));

	FE_GL_ERROR(glBindVertexArray(planeMesh->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, planeMesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	brushVisualShader->stop();
	brushVisualFB->unBind();

	brushVisualFBCleared = false;
}
// **************************** TERRAIN EDITOR TOOLS END ****************************