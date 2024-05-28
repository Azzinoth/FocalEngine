#pragma once

#include "FEObjLoader.h"

namespace FocalEngine
{
	class FEResourceManager;

	struct GLTFBuffer
	{
		std::string Uri;
		int ByteLength = -1;
		char* RawData = nullptr;
	};

	struct GLTFBufferView
	{
		int Buffer = -1;
		int ByteLength = -1;
		int ByteOffset = -1;
		int Target = -1;
	};

	struct GLTFAccessor
	{
		int BufferView = -1;
		int ByteOffset = -1;
		int ComponentType = -1;
		int Count = -1;
		std::vector<int> Max;
		std::vector<int> Min;

		std::string Type;
	};

	struct GLTFPrimitiveRawData
	{
		std::string Name;
		std::vector<float> Positions;
		std::vector<float> Normals;
		std::vector<float> Tangents;
		std::vector<std::vector<float>> UVs;
		std::vector<int> Indices;
	};

	struct GLTFPrimitive
	{
		std::unordered_map<std::string, int> Attributes;
		int Indices = -1;
		int Material = -1;
		int Mode = -1;

		GLTFPrimitiveRawData RawData;
	};

	struct GLTFMesh
	{
		std::string Name;
		std::vector<GLTFPrimitive> Primitives;
	};

	struct GLTFMaterialTexture
	{
		int Index = -1;
		int TexCoord = -1;
		int Scale = 1;
	};

	struct GLTFMaterial
	{
		std::string Name;

		glm::vec4 BaseColor = glm::vec4(-1);
		GLTFMaterialTexture BaseColorTexture;

		GLTFMaterialTexture MetallicRoughnessTexture;
		GLTFMaterialTexture NormalTexture;
		GLTFMaterialTexture OcclusionTexture;
	};

	struct GLTFGameModel
	{
		std::string Name;
		int Primitive = -1;
		int Material = -1;

		int MeshParent = -1;
	};

	struct GLTFNodes
	{
		std::string Name;
		int Mesh = -1;
		glm::vec3 Translation = glm::vec3(0);
		glm::quat Rotation = glm::quat();
		glm::vec3 Scale = glm::vec3(1);
	};

	class FEGLTFLoader
	{
		friend FEResourceManager;
	public:
		SINGLETON_PUBLIC_PART(FEGLTFLoader)

		void Load(const char* FileName);
		void Clear();
		
	private:
		SINGLETON_PRIVATE_PART(FEGLTFLoader)

		std::vector<GLTFBuffer> Buffers;
		std::vector<GLTFBufferView> BufferViews;
		std::vector<GLTFAccessor> Accessors;
		std::vector<GLTFMesh> Meshes;
		std::vector<GLTFNodes> Nodes;

		void LoadPrimitive(Json::Value JsonPrimitive, GLTFPrimitive& NewPrimitive);
		bool LoadMeshRawData(GLTFPrimitive& Primitive);
		bool LoadPositions(GLTFPrimitive& Primitive);
		bool LoadNomals(GLTFPrimitive& Primitive);
		bool LoadTangents(GLTFPrimitive& Primitive);
		bool LoadUV(GLTFPrimitive& Primitive);
		bool LoadIndices(GLTFPrimitive& Primitive);

		GLTFMaterialTexture LoadMaterialTexture(Json::Value JsonTextureNode);

		std::vector<std::string> Images;
		std::vector<std::string> Textures;
		std::vector<GLTFMaterial> Materials;
	};
}