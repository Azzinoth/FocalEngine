#include "FEGLTFLoader.h"
using namespace FocalEngine;

FEGLTFLoader::FEGLTFLoader()
{
    
}

FEGLTFLoader::~FEGLTFLoader()
{
   
}

void FEGLTFLoader::Clear()
{
	Buffers.clear();
	BufferViews.clear();
	Accessors.clear();
	Meshes.clear();

	Images.clear();
	TextureSamplers.clear();
	Textures.clear();
	Materials.clear();

	Nodes.clear();
	Scenes.clear();
	Scene = -1;
}

void FEGLTFLoader::LoadPrimitive(Json::Value JsonPrimitive, GLTFPrimitive& NewPrimitive)
{
	if (JsonPrimitive.isMember("attributes"))
	{
		if (JsonPrimitive["attributes"].isMember("POSITION"))
			NewPrimitive.Attributes["POSITION"] = JsonPrimitive["attributes"]["POSITION"].asInt();

		if (JsonPrimitive["attributes"].isMember("NORMAL"))
			NewPrimitive.Attributes["NORMAL"] = JsonPrimitive["attributes"]["NORMAL"].asInt();

		if (JsonPrimitive["attributes"].isMember("TANGENT"))
			NewPrimitive.Attributes["TANGENT"] = JsonPrimitive["attributes"]["TANGENT"].asInt();

		std::string AttributeName;

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("TEXCOORD_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("COLOR_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("JOINTS_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}

		for (size_t i = 0; i < 16; i++)
		{
			AttributeName = std::string("WEIGHTS_") + std::to_string(i);
			if (JsonPrimitive["attributes"].isMember(AttributeName.c_str()))
			{
				NewPrimitive.Attributes[AttributeName] = JsonPrimitive["attributes"][AttributeName].asInt();
			}
			else
			{
				break;
			}
		}
	}

	if (JsonPrimitive.isMember("indices"))
		NewPrimitive.Indices = JsonPrimitive["indices"].asInt();

	if (JsonPrimitive.isMember("material"))
		NewPrimitive.Material = JsonPrimitive["material"].asInt();

	if (JsonPrimitive.isMember("mode"))
	{
		NewPrimitive.Mode = JsonPrimitive["mode"].asInt();
		if (NewPrimitive.Mode < 0 || NewPrimitive.Mode > 6)
		{
			LOG.Add("Mode is not in range 0-6 in function FEGLTFLoader::LoadPrimitive.", "FE_LOG_LOADING", FE_LOG_ERROR);
		}

		if (NewPrimitive.Mode != 4)
		{
			LOG.Add("Mesh.Primitive.Mode is not TRIANGLES, and is not supported, function FEGLTFLoader::LoadPrimitive.", "FE_LOG_LOADING", FE_LOG_ERROR);
		}
	}

	LoadMeshRawData(NewPrimitive);
}

GLTFTextureInfo FEGLTFLoader::LoadTextureInfo(Json::Value JsonTextureNode)
{
	GLTFTextureInfo TempTextureInfo;

	if (JsonTextureNode.isMember("index"))
	{
		TempTextureInfo.Index = JsonTextureNode["index"].asInt();
	}
	else
	{
		LOG.Add("Index is not present in function FEGLTFLoader::LoadTextureInfo.", "FE_LOG_LOADING", FE_LOG_ERROR);
	}

	if (JsonTextureNode.isMember("texCoord"))
	{
		TempTextureInfo.TexCoord = JsonTextureNode["texCoord"].asInt();
	}

	return TempTextureInfo;
}

void FEGLTFLoader::Load(const char* FileName)
{
	Clear();

	std::ifstream GLTFFile;
	GLTFFile.open(FileName);

	std::string FileData((std::istreambuf_iterator<char>(GLTFFile)), std::istreambuf_iterator<char>());
	GLTFFile.close();

	Json::Value Root;
	JSONCPP_STRING Err;
	Json::CharReaderBuilder Builder;

	const std::unique_ptr<Json::CharReader> Reader(Builder.newCharReader());
	if (!Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Err))
		return;

	std::string Directory = FILE_SYSTEM.GetDirectoryPath(FileName);
	if (Root.isMember("buffers"))
	{
		Json::Value JsonBuffers = Root["buffers"];
		for (size_t i = 0; i < JsonBuffers.size(); i++)
		{
			GLTFBuffer CurrentBuffer;

			if (JsonBuffers[static_cast<int>(i)].isMember("uri"))
				CurrentBuffer.Uri = JsonBuffers[static_cast<int>(i)]["uri"].asString();

			std::fstream File;
			File.open(Directory + "\\" + CurrentBuffer.Uri, std::ios::in | std::ios::binary | std::ios::ate);
			std::streamsize FileSize = File.tellg();
			if (FileSize < 0)
			{
				LOG.Add(std::string("Can't load buffer from: ") + CurrentBuffer.Uri + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
			else
			{
				File.seekg(0, std::ios::beg);
				CurrentBuffer.RawData = new char[static_cast<int>(FileSize)];
				File.read(CurrentBuffer.RawData, FileSize);
			}
			File.close();

			// ByteLength is required.
			if (!JsonBuffers[static_cast<int>(i)].isMember("byteLength"))
			{
				LOG.Add("ByteLength is not present in buffer with index: " + std::to_string(i) + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				return;
			}
			CurrentBuffer.ByteLength = JsonBuffers[static_cast<int>(i)]["byteLength"].asInt();

			if (CurrentBuffer.ByteLength != FileSize)
				LOG.Add("ByteLength and fileSize is not equal in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);

			if (JsonBuffers[static_cast<int>(i)].isMember("name"))
				CurrentBuffer.Name = JsonBuffers[static_cast<int>(i)]["name"].asCString();

			Buffers.push_back(CurrentBuffer);
		}
	}

	if (Root.isMember("bufferViews"))
	{
		Json::Value JsonBufferViews = Root["bufferViews"];
		for (size_t i = 0; i < JsonBufferViews.size(); i++)
		{
			GLTFBufferView CurrentBufferView;

			// Buffer is required.
			if (!JsonBufferViews[static_cast<int>(i)].isMember("buffer"))
			{
				LOG.Add("Buffer is not present in bufferView with index: " + std::to_string(i) + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				return;
			}
			CurrentBufferView.Buffer = JsonBufferViews[static_cast<int>(i)]["buffer"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("byteOffset"))
				CurrentBufferView.ByteOffset = JsonBufferViews[static_cast<int>(i)]["byteOffset"].asInt();

			// ByteLength is required.
			if (!JsonBufferViews[static_cast<int>(i)].isMember("byteLength"))
			{
				LOG.Add("ByteLength is not present in bufferView with index: " + std::to_string(i) + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				return;
			}
			CurrentBufferView.ByteLength = JsonBufferViews[static_cast<int>(i)]["byteLength"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("byteStride"))
				CurrentBufferView.ByteStride = JsonBufferViews[static_cast<int>(i)]["byteStride"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("target"))
				CurrentBufferView.Target = JsonBufferViews[static_cast<int>(i)]["target"].asInt();

			if (JsonBufferViews[static_cast<int>(i)].isMember("name"))
				CurrentBufferView.Name = JsonBufferViews[static_cast<int>(i)]["name"].asCString();

			BufferViews.push_back(CurrentBufferView);
		}
	}

	if (Root.isMember("accessors"))
	{
		Json::Value JsonAccessors = Root["accessors"];
		for (size_t i = 0; i < JsonAccessors.size(); i++)
		{
			GLTFAccessor CurrentAccessor;

			if (JsonAccessors[static_cast<int>(i)].isMember("bufferView"))
				CurrentAccessor.BufferView = JsonAccessors[static_cast<int>(i)]["bufferView"].asInt();

			if (JsonAccessors[static_cast<int>(i)].isMember("byteOffset"))
				CurrentAccessor.ByteOffset = JsonAccessors[static_cast<int>(i)]["byteOffset"].asInt();

			// ComponentType is required.
			if (!JsonAccessors[static_cast<int>(i)].isMember("componentType"))
			{
				LOG.Add("ComponentType is not present in accessor with index: " + std::to_string(i) + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				return;
			}
			CurrentAccessor.ComponentType = JsonAccessors[static_cast<int>(i)]["componentType"].asInt();

			if (JsonAccessors[static_cast<int>(i)].isMember("normalized"))
				CurrentAccessor.bNormalized = JsonAccessors[static_cast<int>(i)]["normalized"].asBool();

			// Count is required.
			if (!JsonAccessors[static_cast<int>(i)].isMember("count"))
			{
				LOG.Add("Count is not present in accessor with index: " + std::to_string(i) + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				return;
			}
			CurrentAccessor.Count = JsonAccessors[static_cast<int>(i)]["count"].asInt();

			// Type is required.
			if (!JsonAccessors[static_cast<int>(i)].isMember("type"))
			{
				LOG.Add("Type is not present in accessor with index: " + std::to_string(i) + " in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
			}
			CurrentAccessor.Type = JsonAccessors[static_cast<int>(i)]["type"].asString();

			if (JsonAccessors[static_cast<int>(i)].isMember("max"))
			{
				/*Json::Value JsonMax = JsonAccessors[static_cast<int>(i)]["max"];
				for (size_t j = 0; j < JsonMax.size(); j++)
				{
					CurrentAccessor.Max.push_back(JsonMax[static_cast<int>(j)].asInt());
				}*/
			}

			if (JsonAccessors[static_cast<int>(i)].isMember("min"))
			{
				/*Json::Value JsonMin = JsonAccessors[static_cast<int>(i)]["min"];
				for (size_t j = 0; j < JsonMin.size(); j++)
				{
					CurrentAccessor.Min.push_back(JsonMin[static_cast<int>(j)].asInt());
				}*/
			}

			
			// read GLTFAccessorSparse

			if (JsonAccessors[static_cast<int>(i)].isMember("name"))
				CurrentAccessor.Name = JsonAccessors[static_cast<int>(i)]["name"].asCString();



			Accessors.push_back(CurrentAccessor);
		}
	}

	if (Root.isMember("meshes"))
	{
		Json::Value JsonMeshes = Root["meshes"];
		for (size_t i = 0; i < JsonMeshes.size(); i++)
		{
			GLTFMesh CurrentMesh;
			if (JsonMeshes[static_cast<int>(i)].isMember("name"))
				CurrentMesh.Name = JsonMeshes[static_cast<int>(i)]["name"].asCString();

			if (JsonMeshes[static_cast<int>(i)].isMember("primitives"))
			{
				Json::Value JsonPrimitives = JsonMeshes[static_cast<int>(i)]["primitives"];
				for (size_t j = 0; j < JsonPrimitives.size(); j++)
				{
					GLTFPrimitive CurrentPrimitive;
					LoadPrimitive(JsonPrimitives[static_cast<int>(j)], CurrentPrimitive);
					CurrentMesh.Primitives.push_back(CurrentPrimitive);
				}
			}

			Meshes.push_back(CurrentMesh);
		}
	}

	if (Root.isMember("images"))
	{
		Json::Value JsonImages = Root["images"];
		for (size_t i = 0; i < JsonImages.size(); i++)
		{
			GLTFImage NewImage;

			if (JsonImages[static_cast<int>(i)].isMember("uri"))
				NewImage.Uri = JsonImages[static_cast<int>(i)]["uri"].asCString();

			if (JsonImages[static_cast<int>(i)].isMember("mimeType"))
				NewImage.MimeType = JsonImages[static_cast<int>(i)]["mimeType"].asCString();

			if (JsonImages[static_cast<int>(i)].isMember("bufferView"))
				NewImage.BufferView = JsonImages[static_cast<int>(i)]["bufferView"].asInt();

			if (JsonImages[static_cast<int>(i)].isMember("name"))
				NewImage.Name = JsonImages[static_cast<int>(i)]["name"].asCString();

			Images.push_back(NewImage);
		}
	}

	if (Root.isMember("textures"))
	{
		Json::Value JsonTextures = Root["textures"];
		for (size_t i = 0; i < JsonTextures.size(); i++)
		{
			GLTFTexture NewTexture;

			if (JsonTextures[static_cast<int>(i)].isMember("sampler"))
				NewTexture.Sampler = JsonTextures[static_cast<int>(i)]["sampler"].asInt();

			if (JsonTextures[static_cast<int>(i)].isMember("source"))
				NewTexture.Source = JsonTextures[static_cast<int>(i)]["source"].asInt();

			if (JsonTextures[static_cast<int>(i)].isMember("name"))
				NewTexture.Name = JsonTextures[static_cast<int>(i)]["name"].asCString();

			Textures.push_back(NewTexture);
		}
	}

	if (Root.isMember("samplers"))
	{
		Json::Value JsonSamplers = Root["samplers"];
		for (size_t i = 0; i < JsonSamplers.size(); i++)
		{
			GLTFTextureSampler NewSampler;

			if (JsonSamplers[static_cast<int>(i)].isMember("magFilter"))
				NewSampler.MagFilter = JsonSamplers[static_cast<int>(i)]["magFilter"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("minFilter"))
				NewSampler.MinFilter = JsonSamplers[static_cast<int>(i)]["minFilter"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("wrapS"))
				NewSampler.WrapS = JsonSamplers[static_cast<int>(i)]["wrapS"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("wrapT"))
				NewSampler.WrapT = JsonSamplers[static_cast<int>(i)]["wrapT"].asInt();

			if (JsonSamplers[static_cast<int>(i)].isMember("name"))
				NewSampler.Name = JsonSamplers[static_cast<int>(i)]["name"].asCString();

			TextureSamplers.push_back(NewSampler);
		}
	}

	if (Root.isMember("materials"))
	{
		Json::Value JsonMaterials = Root["materials"];
		for (size_t i = 0; i < JsonMaterials.size(); i++)
		{
			GLTFMaterial NewMaterial;

			if (JsonMaterials[static_cast<int>(i)].isMember("name"))
				NewMaterial.Name = JsonMaterials[static_cast<int>(i)]["name"].asCString();

			if (JsonMaterials[static_cast<int>(i)].isMember("pbrMetallicRoughness"))
			{
				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("baseColorFactor"))
				{
					NewMaterial.PBRMetallicRoughness.BaseColorFactor = glm::vec4(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][0].asDouble(),
																				 JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][1].asDouble(),
																				 JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][2].asDouble(),
																				 JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorFactor"][3].asDouble());
				}

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("baseColorTexture"))
					NewMaterial.PBRMetallicRoughness.BaseColorTexture = LoadTextureInfo(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["baseColorTexture"]);
				

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("metallicFactor"))
					NewMaterial.PBRMetallicRoughness.MetallicFactor = static_cast<float>(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["metallicFactor"].asDouble());

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("roughnessFactor"))
					NewMaterial.PBRMetallicRoughness.RoughnessFactor = static_cast<float>(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["roughnessFactor"].asDouble());

				if (JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"].isMember("metallicRoughnessTexture"))
					NewMaterial.PBRMetallicRoughness.MetallicRoughnessTexture = LoadTextureInfo(JsonMaterials[static_cast<int>(i)]["pbrMetallicRoughness"]["metallicRoughnessTexture"]);
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("normalTexture"))
			{
				if (JsonMaterials[int(i)]["normalTexture"].isMember("index"))
				{
					NewMaterial.NormalTexture.Index = JsonMaterials[int(i)]["normalTexture"]["index"].asInt();

					if (JsonMaterials[int(i)]["normalTexture"].isMember("texCoord"))
						NewMaterial.NormalTexture.TexCoord = JsonMaterials[int(i)]["normalTexture"]["texCoord"].asInt();

					if (JsonMaterials[int(i)]["normalTexture"].isMember("scale"))
						NewMaterial.NormalTexture.Scale = JsonMaterials[int(i)]["normalTexture"]["scale"].asFloat();
				}
				else
				{
					LOG.Add("NormalTexture.Index is not present in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("occlusionTexture"))
			{
				if (JsonMaterials[int(i)]["occlusionTexture"].isMember("index"))
				{
					NewMaterial.OcclusionTexture.Index = JsonMaterials[int(i)]["occlusionTexture"]["index"].asInt();

					if (JsonMaterials[int(i)]["occlusionTexture"].isMember("texCoord"))
						NewMaterial.OcclusionTexture.TexCoord = JsonMaterials[int(i)]["occlusionTexture"]["texCoord"].asInt();

					if (JsonMaterials[int(i)]["occlusionTexture"].isMember("strength"))
						NewMaterial.OcclusionTexture.Strength = JsonMaterials[int(i)]["occlusionTexture"]["strength"].asFloat();
				}
				else
				{
					LOG.Add("OcclusionTexture.Index is not present in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("emissiveTexture"))
				NewMaterial.EmissiveTexture = LoadTextureInfo(JsonMaterials[static_cast<int>(i)]["emissiveTexture"]);
			
			if (JsonMaterials[static_cast<int>(i)].isMember("emissiveFactor"))
			{
				NewMaterial.EmissiveFactor = glm::vec3(JsonMaterials[static_cast<int>(i)]["emissiveFactor"][0].asDouble(),
													   JsonMaterials[static_cast<int>(i)]["emissiveFactor"][1].asDouble(),
													   JsonMaterials[static_cast<int>(i)]["emissiveFactor"][2].asDouble());
			}

			if (JsonMaterials[static_cast<int>(i)].isMember("alphaMode"))
				NewMaterial.AlphaMode = JsonMaterials[static_cast<int>(i)]["alphaMode"].asCString();

			if (JsonMaterials[static_cast<int>(i)].isMember("alphaCutoff"))
				NewMaterial.AlphaCutoff = static_cast<float>(JsonMaterials[static_cast<int>(i)]["alphaCutoff"].asDouble());

			if (JsonMaterials[static_cast<int>(i)].isMember("doubleSided"))
				NewMaterial.bDoubleSided = JsonMaterials[static_cast<int>(i)]["doubleSided"].asBool();

			Materials.push_back(NewMaterial);
		}
	}

	if (Root.isMember("nodes"))
	{
		Json::Value JsonNodes = Root["nodes"];
		for (size_t i = 0; i < JsonNodes.size(); i++)
		{
			GLTFNodes NewNode;
			bool bAnyInfoWasRead = false;

			if (JsonNodes[static_cast<int>(i)].isMember("name"))
			{
				NewNode.Name = JsonNodes[static_cast<int>(i)]["name"].asCString();
				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("camera"))
			{
				int CameraIndex = JsonNodes[static_cast<int>(i)]["camera"].asInt();
				if (CameraIndex < 0)
				{
					LOG.Add("Camera is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					NewNode.Camera = CameraIndex;
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("mesh"))
			{
				int MeshIndex = JsonNodes[static_cast<int>(i)]["mesh"].asInt();
				if (MeshIndex < 0)
				{
					LOG.Add("Mesh is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					NewNode.Mesh = MeshIndex;
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("skin"))
			{
				int SkinIndex = JsonNodes[static_cast<int>(i)]["skin"].asInt();
				if (SkinIndex < 0)
				{
					LOG.Add("Skin is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
					
				}
				else
				{
					NewNode.Skin = SkinIndex;
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("matrix"))
			{
				if (JsonNodes[static_cast<int>(i)]["matrix"].size() != 16)
				{
					LOG.Add("Matrix size is not 16 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
				}
				else
				{
					NewNode.Matrix.resize(16);
					for (int j = 0; j < 16; j++)
					{
						NewNode.Matrix[j] = JsonNodes[static_cast<int>(i)]["matrix"][j].asFloat();
					}
					bAnyInfoWasRead = true;
				}
			}

			if (JsonNodes[static_cast<int>(i)].isMember("translation"))
			{
				NewNode.Translation = glm::vec3(JsonNodes[static_cast<int>(i)]["translation"][0].asDouble(),
												JsonNodes[static_cast<int>(i)]["translation"][1].asDouble(),
											    JsonNodes[static_cast<int>(i)]["translation"][2].asDouble());

				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("rotation"))
			{
				// glm::quat takes w,x,y,z. But gltf stores x,y,z,w.
				NewNode.Rotation = glm::quat(JsonNodes[static_cast<int>(i)]["rotation"][3].asFloat(),
											 JsonNodes[static_cast<int>(i)]["rotation"][0].asFloat(),
											 JsonNodes[static_cast<int>(i)]["rotation"][1].asFloat(),
											 JsonNodes[static_cast<int>(i)]["rotation"][2].asFloat());

				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("scale"))
			{
				NewNode.Scale = glm::vec3(JsonNodes[static_cast<int>(i)]["scale"][0].asDouble(),
										  JsonNodes[static_cast<int>(i)]["scale"][1].asDouble(),
										  JsonNodes[static_cast<int>(i)]["scale"][2].asDouble());

				bAnyInfoWasRead = true;
			}

			if (JsonNodes[static_cast<int>(i)].isMember("children"))
			{
				std::unordered_map<int, bool> ChildrensPresent;

				Json::Value JsonChildrens = JsonNodes[static_cast<int>(i)]["children"];
				for (size_t j = 0; j < JsonChildrens.size(); j++)
				{
					int NewChild = JsonChildrens[static_cast<int>(j)].asInt();
					// Each element in the array MUST be greater than or equal to 0.
					if (NewChild < 0)
					{
						LOG.Add("Child is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
						continue;
					}

					// Each element in the array MUST be unique.
					if (ChildrensPresent.find(NewChild) != ChildrensPresent.end())
					{
						LOG.Add("Child is not unique in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
						continue;
					}

					ChildrensPresent[NewChild] = true;
					NewNode.Children.push_back(NewChild);
				}
			}

			if (bAnyInfoWasRead)
				Nodes.push_back(NewNode);
		}
	}

	if (Root.isMember("scenes"))
	{
		Json::Value JsonScenes = Root["scenes"];
		for (size_t i = 0; i < JsonScenes.size(); i++)
		{
			GLTFScene NewScene;

			if (JsonScenes[static_cast<int>(i)].isMember("name"))
				NewScene.Name = JsonScenes[static_cast<int>(i)]["name"].asCString();

			if (JsonScenes[static_cast<int>(i)].isMember("nodes"))
			{
				Json::Value JsonNodeIndexes = JsonScenes[static_cast<int>(i)]["nodes"];
				for (size_t j = 0; j < JsonNodeIndexes.size(); j++)
				{
					int NewNodeIndex = JsonNodeIndexes[static_cast<int>(j)].asInt();
					// Each element in the array MUST be greater than or equal to 0.
					if (NewNodeIndex < 0)
					{
						LOG.Add("Node is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
						continue;
					}

					NewScene.RootChildren.push_back(NewNodeIndex);
				}
			}

			Scenes.push_back(NewScene);
		}
	}

	if (Root.isMember("scene"))
	{
		int SceneIndex = Root["scene"].asInt();
		if (SceneIndex < 0)
		{
			LOG.Add("Scene is less than 0 in function FEGLTFLoader::Load.", "FE_LOG_LOADING", FE_LOG_ERROR);
		}
		else
		{
			Scene = Root["scene"].asInt();
		}
	}
}

bool FEGLTFLoader::LoadPositions(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Attributes["POSITION"])
	{
		LOG.Add("Primitive.Attributes[\"POSITION\"] is out of bounds of accessors.size() function FEGLTFLoader::LoadPositions.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Attributes["POSITION"];
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5126)
	{
		LOG.Add("ComponentType is not float in function FEGLTFLoader::LoadPositions.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	/*if (CurrentAccessor.ByteOffset > 0)
		ByteOffset = CurrentAccessor.ByteOffset;*/
	ByteOffset = CurrentBufferView.ByteOffset + CurrentAccessor.ByteOffset;

	// It is vec3 so size should be currentAccessor.count * 3.
	Primitive.RawData.Positions.resize(CurrentAccessor.Count * 3);
	size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
	memcpy_s(Primitive.RawData.Positions.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);

	return true;
}

bool FEGLTFLoader::LoadNomals(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Attributes["NORMAL"])
	{
		LOG.Add("Primitive.Attributes[\"NORMAL\"] is out of bounds of accessors.size() function FEGLTFLoader::LoadNomals.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Attributes["NORMAL"];
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5126)
	{
		LOG.Add("ComponentType is not float in function FEGLTFLoader::LoadNomals.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	//if (CurrentAccessor.ByteOffset > 0)
		//ByteOffset = CurrentAccessor.ByteOffset;
	ByteOffset = CurrentBufferView.ByteOffset + CurrentAccessor.ByteOffset;

	// It is vec3 so size should be currentAccessor.count * 3.
	Primitive.RawData.Normals.resize(CurrentAccessor.Count * 3);
	size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
	memcpy_s(Primitive.RawData.Normals.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);

	return true;
}

bool FEGLTFLoader::LoadTangents(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Attributes["TANGENT"])
	{
		LOG.Add("Primitive.Attributes[\"TANGENT\"] is out of bounds of accessors.size() function FEGLTFLoader::LoadTangents.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Attributes["TANGENT"];
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5126)
	{
		LOG.Add("ComponentType is not float in function FEGLTFLoader::LoadTangents.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	/*if (CurrentAccessor.ByteOffset > 0)
		ByteOffset = CurrentAccessor.ByteOffset;*/
	ByteOffset = CurrentBufferView.ByteOffset + CurrentAccessor.ByteOffset;

	if (CurrentAccessor.Type == "VEC3")
	{
		Primitive.RawData.Tangents.resize(CurrentAccessor.Count * 3);
		size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
		memcpy_s(Primitive.RawData.Tangents.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);
	}
	else if (CurrentAccessor.Type == "VEC4")
	{
		std::vector<float> TempBuffer;
		TempBuffer.resize(CurrentAccessor.Count * 4);
		size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
		memcpy_s(TempBuffer.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);

		int iteration = 0;
		for (size_t i = 0; i < TempBuffer.size(); i++)
		{
			iteration++;
			if (iteration == 4)
			{
				iteration = 0;
				continue;
			}
			Primitive.RawData.Tangents.push_back(TempBuffer[i]);
		}
	}
	else
	{
		return false;
	}

	return true;
}

bool FEGLTFLoader::LoadUV(GLTFPrimitive& Primitive)
{
	for (size_t i = 0; i < 16; i++)
	{
		std::string AttributeName = std::string("TEXCOORD_") + std::to_string(i);
		if (Primitive.Attributes.find(AttributeName) != Primitive.Attributes.end())
		{
			if (Accessors.size() <= Primitive.Attributes[AttributeName])
			{
				LOG.Add("Primitive.Attributes[" + AttributeName + "] is out of bounds of accessors.size() function FEGLTFLoader::LoadUV.", "FE_LOG_LOADING", FE_LOG_ERROR);
				//return false;
			}

			const int AccessorID = Primitive.Attributes[AttributeName];
			const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
			const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
			const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

			if (CurrentAccessor.ComponentType != 5126)
			{
				LOG.Add("ComponentType is not float in function FEGLTFLoader::LoadUV.", "FE_LOG_LOADING", FE_LOG_ERROR);
				//return false;
			}

			int ByteOffset = 0;
			//if (CurrentAccessor.ByteOffset > 0)
			//	ByteOffset = CurrentAccessor.ByteOffset;
			ByteOffset = CurrentBufferView.ByteOffset + CurrentAccessor.ByteOffset;

			Primitive.RawData.UVs.resize(Primitive.RawData.UVs.size() + 1);
			// It is vec2 so size should be currentAccessor.count * 2.
			Primitive.RawData.UVs[i].resize(CurrentAccessor.Count * 2);
			size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
			memcpy_s(Primitive.RawData.UVs[i].data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);
		}
	}

	return true;
}

bool FEGLTFLoader::LoadIndices(GLTFPrimitive& Primitive)
{
	if (Accessors.size() <= Primitive.Indices)
	{
		LOG.Add("Primitive.Indices is out of bounds of accessors.size() function FEGLTFLoader::LoadIndices.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	const int AccessorID = Primitive.Indices;
	const GLTFAccessor CurrentAccessor = Accessors[AccessorID];
	const GLTFBufferView CurrentBufferView = BufferViews[CurrentAccessor.BufferView];
	const GLTFBuffer CurrentBuffer = Buffers[CurrentBufferView.Buffer];

	if (CurrentAccessor.ComponentType != 5121 && CurrentAccessor.ComponentType != 5123 && CurrentAccessor.ComponentType != 5125)
	{
		LOG.Add("ComponentType is not unsigned byte, unsigned short or unsigned int in function FEGLTFLoader::LoadIndices.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return false;
	}

	int ByteOffset = 0;
	//if (CurrentAccessor.ByteOffset > 0)
	//	ByteOffset = CurrentAccessor.ByteOffset;
	ByteOffset = CurrentBufferView.ByteOffset + CurrentAccessor.ByteOffset;

	if (CurrentAccessor.ComponentType == 5121)
	{
		std::vector<unsigned char> TempBuffer;
		TempBuffer.resize(CurrentAccessor.Count);
		size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
		memcpy_s(TempBuffer.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);

		for (size_t i = 0; i < TempBuffer.size(); i++)
		{
			Primitive.RawData.Indices.push_back((int)TempBuffer[i]);
		}
	}
	else if (CurrentAccessor.ComponentType == 5123)
	{
		std::vector<unsigned short> TempBuffer;
		TempBuffer.resize(CurrentAccessor.Count);
		size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
		memcpy_s(TempBuffer.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);

		for (size_t i = 0; i < TempBuffer.size(); i++)
		{
			Primitive.RawData.Indices.push_back((int)TempBuffer[i]);
		}
	}
	else if (CurrentAccessor.ComponentType == 5125)
	{
		std::vector<unsigned int> TempBuffer;
		TempBuffer.resize(CurrentAccessor.Count);
		size_t TotalSize = GetTotalMemorySize(CurrentAccessor.Count, CurrentAccessor.ComponentType, CurrentAccessor.Type);
		memcpy_s(TempBuffer.data(), TotalSize, reinterpret_cast<void*>(CurrentBuffer.RawData + ByteOffset), TotalSize);

		for (size_t i = 0; i < TempBuffer.size(); i++)
		{
			Primitive.RawData.Indices.push_back((int)TempBuffer[i]);
		}
	}

	return true;
}

bool FEGLTFLoader::LoadMeshRawData(GLTFPrimitive& Primitive)
{
	if (Primitive.Attributes.find("POSITION") != Primitive.Attributes.end())
	{
		if (!LoadPositions(Primitive))
			return false;
	}

	if (Primitive.Attributes.find("NORMAL") != Primitive.Attributes.end())
	{
		if (!LoadNomals(Primitive))
			return false;
	}

	if (Primitive.Attributes.find("TANGENT") != Primitive.Attributes.end())
	{
		if (!LoadTangents(Primitive))
			return false;
	}

	if (Primitive.Attributes.find("TEXCOORD_0") != Primitive.Attributes.end())
	{
		if (!LoadUV(Primitive))
			return false;
	}

	if (Primitive.Indices != -1)
	{
		if (!LoadIndices(Primitive))
			return false;
	}

	return true;
}

size_t FEGLTFLoader::GetComponentSize(const int ComponentType)
{
	switch (ComponentType)
	{
		case 5120: return sizeof(int8_t);   // BYTE
		case 5121: return sizeof(uint8_t);  // UNSIGNED_BYTE
		case 5122: return sizeof(int16_t);  // SHORT
		case 5123: return sizeof(uint16_t); // UNSIGNED_SHORT
		case 5125: return sizeof(uint32_t); // UNSIGNED_INT
		case 5126: return sizeof(float);    // FLOAT
		default: return 0;
	}
}

size_t FEGLTFLoader::GetComponentCount(const std::string& Type)
{
	if (Type == "SCALAR") return 1;
	if (Type == "VEC2") return 2;
	if (Type == "VEC3") return 3;
	if (Type == "VEC4") return 4;
	if (Type == "MAT2") return 4;
	if (Type == "MAT3") return 9;
	if (Type == "MAT4") return 16;
	return 0;
}

size_t FEGLTFLoader::GetTotalMemorySize(const int Count, const int ComponentType, const std::string& Type)
{
	return Count * GetComponentSize(ComponentType) * GetComponentCount(Type);
}