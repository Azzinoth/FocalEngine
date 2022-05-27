#pragma once

#include "FEObjLoader.h"

namespace FocalEngine
{
	class FEResourceManager;

	struct glTFBuffer
	{
		std::string uri;
		int byteLength = -1;
		char* rawData = nullptr;
	};

	struct glTFBufferView
	{
		int buffer = -1;
		int byteLength = -1;
		int byteOffset = -1;
		int target = -1;
	};

	struct glTFAccessor
	{
		int bufferView = -1;
		int byteOffset = -1;
		int componentType = -1;
		int count = -1;
		std::vector<int> max;
		std::vector<int> min;

		std::string type;
	};

	struct glTFPrimitiveRawData
	{
		std::string name;
		std::vector<float> positions;
		std::vector<float> normals;
		std::vector<float> tangents;
		std::vector<std::vector<float>> UVs;
		std::vector<int> indices;
	};

	struct glTFPrimitive
	{
		std::unordered_map<std::string, int> attributes;
		int indices = -1;
		int material = -1;
		int mode = -1;

		glTFPrimitiveRawData rawData;
	};

	struct glTFMesh
	{
		std::string name;
		std::vector<glTFPrimitive> primitives;
	};

	struct glTFMaterialTexture
	{
		int index = -1;
		int texCoord = -1;
		int scale = 1;
	};

	struct glTFMaterial
	{
		std::string name;

		glm::vec4 baseColor = glm::vec4(-1);
		glTFMaterialTexture baseColorTexture;

		glTFMaterialTexture metallicRoughnessTexture;
		glTFMaterialTexture normalTexture;
		glTFMaterialTexture occlusionTexture;
	};

	struct glTFGameModel
	{
		std::string name;
		int primitive = -1;
		int material = -1;

		int meshParent = -1;
	};

	struct glTFEntity
	{
		std::string name;
		int mesh = -1;
		glm::vec3 translation = glm::vec3(0);
		glm::vec4 rotation = glm::vec4(0);
		glm::vec3 scale = glm::vec3(1);
	};

	class FEglTFLoader
	{
		friend FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEglTFLoader)

		void load(const char* fileName);
		void clear();
		
	private:
		SINGLETON_PRIVATE_PART(FEglTFLoader)

		std::vector<glTFBuffer> buffers;
		std::vector<glTFBufferView> bufferViews;
		std::vector<glTFAccessor> accessors;
		std::vector<glTFPrimitive> primitives;
		std::vector<glTFMesh> meshes;
		std::vector<glTFEntity> entities;

		void loadPrimitive(Json::Value jsonPrimitive, glTFPrimitive& newPrimitive);
		bool loadMeshRawData(glTFPrimitive& primitive);
		bool loadPositions(glTFPrimitive& primitive);
		bool loadNomals(glTFPrimitive& primitive);
		bool loadTangents(glTFPrimitive& primitive);
		bool loadUV(glTFPrimitive& primitive);
		bool loadIndices(glTFPrimitive& primitive);

		glTFMaterialTexture loadMaterialTexture(Json::Value jsonTextureNode);

		std::vector<std::string> images;
		std::vector<std::string> textures;
		std::vector<glTFMaterial> materials;
		std::vector<glTFGameModel> gameModels;
	};
}