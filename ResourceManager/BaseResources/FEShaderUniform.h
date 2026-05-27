#pragma once

#include "FETexture.h"
#include <any>
#include <typeindex>

namespace FocalEngine
{
	enum class FE_SHADER_UNIFORM_TYPE
	{
		FE_NULL = 0,
		FE_BOOL = 1,
		FE_BOOL_ARRAY = 2,
		FE_UNSIGNED_INT = 3,
		FE_UNSIGNED_INT_ARRAY = 4,
		FE_INT = 5,
		FE_INT_ARRAY = 6,
		FE_FLOAT = 7,
		FE_FLOAT_ARRAY = 8,
		FE_FLOAT_VECTOR2 = 9,
		FE_FLOAT_VECTOR2_ARRAY = 10,
		FE_FLOAT_VECTOR3 = 11,
		FE_FLOAT_VECTOR3_ARRAY = 12,
		FE_FLOAT_VECTOR4 = 13,
		FE_FLOAT_VECTOR4_ARRAY = 14,
		FE_MAT4 = 15,
		FE_MAT4_ARRAY = 16,
		FE_SAMPLER_1D = 17,
		FE_SAMPLER_1D_ARRAY = 18,
		FE_SAMPLER_2D = 19,
		FE_SAMPLER_2D_ARRAY = 20,
		FE_SAMPLER_3D = 21,
		FE_SAMPLER_3D_ARRAY = 22,
		FE_SAMPLER_CUBE = 23,
		FE_SAMPLER_CUBE_ARRAY = 24,
		FE_IMAGE_1D = 25,
		FE_IMAGE_1D_ARRAY = 26,
		FE_IMAGE_2D = 27,
		FE_IMAGE_2D_ARRAY = 28,
		FE_IMAGE_3D = 29,
		FE_IMAGE_3D_ARRAY = 30
	};

	struct FEShaderUniformValue
	{
		friend struct FEShaderUniform;
		friend class FEShader;
		friend class FERenderer;

		FEShaderUniformValue();

		template<typename T>
		FEShaderUniformValue(const std::string& Name, const T& Data);

		template<typename T>
		const T& GetValue() const;

		template<typename T>
		bool SetValue(const T& Value);

		std::vector<FE_SHADER_UNIFORM_TYPE> GetCompatibleTypes() const;
		template<typename T>
		bool IsType() const;

		std::string GetName() const;
		void SetName(std::string NewName);
	private:
		std::any Data;
		std::vector<FE_SHADER_UNIFORM_TYPE> CompatibleTypes = { FE_SHADER_UNIFORM_TYPE::FE_NULL };
		std::type_index TypeIndex{ typeid(void) };
		std::string Name = "";

		static const std::unordered_map<FE_SHADER_UNIFORM_TYPE, std::type_index>& GetUniformTypeToCTypeMap()
		{
			static const std::unordered_map<FE_SHADER_UNIFORM_TYPE, std::type_index> Mapping = {
				{FE_SHADER_UNIFORM_TYPE::FE_BOOL, std::type_index(typeid(bool))},
				{FE_SHADER_UNIFORM_TYPE::FE_BOOL_ARRAY, std::type_index(typeid(std::vector<bool>))},
				{FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_INT, std::type_index(typeid(int))},
				{FE_SHADER_UNIFORM_TYPE::FE_INT_ARRAY, std::type_index(typeid(std::vector<int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT, std::type_index(typeid(float))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_ARRAY, std::type_index(typeid(std::vector<float>))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2, std::type_index(typeid(glm::vec2))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2_ARRAY, std::type_index(typeid(std::vector<glm::vec2>))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3, std::type_index(typeid(glm::vec3))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3_ARRAY, std::type_index(typeid(std::vector<glm::vec3>))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4, std::type_index(typeid(glm::vec4))},
				{FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4_ARRAY, std::type_index(typeid(std::vector<glm::vec4>))},
				{FE_SHADER_UNIFORM_TYPE::FE_MAT4, std::type_index(typeid(glm::mat4))},
				{FE_SHADER_UNIFORM_TYPE::FE_MAT4_ARRAY, std::type_index(typeid(std::vector<glm::mat4>))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_1D, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_1D_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_2D, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_2D_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_3D, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_3D_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_CUBE, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_CUBE_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_IMAGE_1D, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_IMAGE_1D_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_IMAGE_2D, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_IMAGE_2D_ARRAY, std::type_index(typeid(std::vector<unsigned int>))},
				{FE_SHADER_UNIFORM_TYPE::FE_IMAGE_3D, std::type_index(typeid(unsigned int))},
				{FE_SHADER_UNIFORM_TYPE::FE_IMAGE_3D_ARRAY, std::type_index(typeid(std::vector<unsigned int>))}
			};

			return Mapping;
		}

		static const std::unordered_map<std::type_index, std::vector<FE_SHADER_UNIFORM_TYPE>>& GetCTypeToUniformTypesMap()
		{
			static const std::unordered_map<std::type_index, std::vector<FE_SHADER_UNIFORM_TYPE>> TypeToEnum = {
				{std::type_index(typeid(bool)), {FE_SHADER_UNIFORM_TYPE::FE_BOOL}},
				{std::type_index(typeid(std::vector<bool>)), {FE_SHADER_UNIFORM_TYPE::FE_BOOL_ARRAY}},
				{std::type_index(typeid(unsigned int)), {FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT,
														 FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_1D,
														 FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_2D,
														 FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_3D,
														 FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_CUBE,
														 FE_SHADER_UNIFORM_TYPE::FE_IMAGE_1D,
														 FE_SHADER_UNIFORM_TYPE::FE_IMAGE_2D,
														 FE_SHADER_UNIFORM_TYPE::FE_IMAGE_3D}},
				{std::type_index(typeid(std::vector<unsigned int>)), {FE_SHADER_UNIFORM_TYPE::FE_UNSIGNED_INT_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_1D_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_2D_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_3D_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_SAMPLER_CUBE_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_IMAGE_1D_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_IMAGE_2D_ARRAY,
																	  FE_SHADER_UNIFORM_TYPE::FE_IMAGE_3D_ARRAY}},
				{std::type_index(typeid(int)), {FE_SHADER_UNIFORM_TYPE::FE_INT}},
				{std::type_index(typeid(std::vector<int>)), {FE_SHADER_UNIFORM_TYPE::FE_INT_ARRAY}},
				{std::type_index(typeid(float)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT}},
				{std::type_index(typeid(std::vector<float>)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_ARRAY}},
				{std::type_index(typeid(glm::vec2)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2}},
				{std::type_index(typeid(std::vector<glm::vec2>)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR2_ARRAY}},
				{std::type_index(typeid(glm::vec3)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3}},
				{std::type_index(typeid(std::vector<glm::vec3>)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR3_ARRAY}},
				{std::type_index(typeid(glm::vec4)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4}},
				{std::type_index(typeid(std::vector<glm::vec4>)), {FE_SHADER_UNIFORM_TYPE::FE_FLOAT_VECTOR4_ARRAY}},
				{std::type_index(typeid(glm::mat4)), {FE_SHADER_UNIFORM_TYPE::FE_MAT4}},
				{std::type_index(typeid(std::vector<glm::mat4>)), {FE_SHADER_UNIFORM_TYPE::FE_MAT4_ARRAY}}
			};

			return TypeToEnum;
		}
	};

	struct FEShaderUniform
	{
		friend class FEShader;
		friend class FEMaterial;
		friend class FERenderer;

		FEShaderUniform();
		~FEShaderUniform();

		template<typename T>
		FEShaderUniform(FE_SHADER_UNIFORM_TYPE Type, const T& DataToInitialize, const std::string& Name, std::vector<GLuint>& Locations);

		template<typename T>
		const T& GetValue() const;

		template<typename T>
		bool SetValue(const T& Value);

		FE_SHADER_UNIFORM_TYPE GetType() const;
		template<typename T>
		bool IsType() const;

		std::string GetName();
		void SetName(std::string NewName);
	private:
		FEShaderUniformValue CurrentValue;

		FE_SHADER_UNIFORM_TYPE Type = FE_SHADER_UNIFORM_TYPE::FE_NULL;
		std::type_index TypeIndex{ typeid(void) };
		std::string Name = "";

		size_t ElementCount = 0;
		std::vector<GLuint> Locations;
	public:
		void LoadUniformToGPU();
	};

#include "FEShaderUniform.inl"
}