#pragma once

#include "FEObjLoader.h"

namespace FocalEngine
{
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

	struct GLTFImage
	{
		std::string Uri;
		std::string MimeType;
		int BufferView = -1;
		std::string Name;
		std::vector<GLTFPrimitive> Primitives;
	};

	struct GLTFTextureSampler
	{
		int MagFilter = -1;
		int MinFilter = -1;
		// Default: REPEAT
		int WrapS = 10497;
		// Default: REPEAT
		int WrapT = 10497;
		std::string Name;
	
	};

	struct GLTFTexture
	{
		int Sampler = -1;
		int Source = -1;
		std::string Name;
	};

	struct GLTFTextureInfo
	{
		int Index = -1;
		int TexCoord = 0;
	};

	struct GLTFMaterialNormalTextureInfo
	{
		int Index = -1;
		int TexCoord = 0;
		int Scale = 1;
	};

	struct GLTFMaterialOcclusionTextureInfo
	{
		int Index = -1;
		int TexCoord = 0;
		int Strength = 1;
	};

	struct GLTFMaterialPBRMetallicRoughness
	{
		glm::vec4 BaseColorFactor = glm::vec4(1);
		GLTFTextureInfo BaseColorTexture;

		float MetallicFactor = 1;
		float RoughnessFactor = 1;
		GLTFTextureInfo MetallicRoughnessTexture;
	};

	struct GLTFMaterial
	{
		std::string Name;

		GLTFMaterialPBRMetallicRoughness PBRMetallicRoughness;

		GLTFMaterialNormalTextureInfo NormalTexture;
		GLTFMaterialOcclusionTextureInfo OcclusionTexture;

		GLTFTextureInfo EmissiveTexture;
		glm::vec3 EmissiveFactor = glm::vec3(-1);

		std::string AlphaMode = "OPAQUE";
		float AlphaCutoff = 0.5f;

		bool bDoubleSided = false;
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
		std::vector<int> Children;

		int Mesh = -1;

		glm::vec3 Translation = glm::vec3(0);
		glm::quat Rotation = glm::quat();
		glm::vec3 Scale = glm::vec3(1);

		// That is alternative to Translation, Rotation and Scale
		// A floating-point 4x4 transformation matrix stored in column-major order.
		std::vector<float> Matrix;

		// The index of the camera referenced by this node.
		int Camera = -1;
		// When the node contains skin, all mesh.primitives MUST contain JOINTS_0 and WEIGHTS_0 attributes.
		int Skin = -1;
	};

	struct GLTFScene
	{
		std::string Name;
		// All nodes listed in scene.nodes array MUST be root nodes, i.e., they MUST NOT be listed in a node.children array of any node. The same root node MAY appear in multiple scenes.
		std::vector<int> RootChildren;
	};

	class FEGLTFLoader
	{
		friend class FEResourceManager;
		friend class FEScene;
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

		std::vector<GLTFImage> Images;
		std::vector<GLTFTextureSampler> TextureSamplers;
		GLTFTextureInfo LoadTextureInfo(Json::Value JsonTextureNode);
		std::vector<GLTFTexture> Textures;
		std::vector<GLTFMaterial> Materials;

		std::vector<GLTFNodes> Nodes;
		// A glTF asset that does not contain any scenes SHOULD be treated as a library of individual entities such as materials or meshes.
		std::vector<GLTFScene> Scenes;

		// Identifies which of the scenes in the array SHOULD be displayed at load time.
		int Scene = -1;

		void LoadPrimitive(Json::Value JsonPrimitive, GLTFPrimitive& NewPrimitive);
		bool LoadMeshRawData(GLTFPrimitive& Primitive);
		bool LoadPositions(GLTFPrimitive& Primitive);
		bool LoadNomals(GLTFPrimitive& Primitive);
		bool LoadTangents(GLTFPrimitive& Primitive);
		bool LoadUV(GLTFPrimitive& Primitive);
		bool LoadIndices(GLTFPrimitive& Primitive);
	};
}