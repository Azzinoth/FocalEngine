#pragma once
#pragma warning (disable: 4752)     // found Intel(R) Advanced Vector Extensions; consider using / arch:AVX	FocalEnginePrivate
#pragma warning (disable: 4334)     // '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?) in lodepng.cpp

#include "FEBasicApplication.h"
#include "FocalEngineAPI.h"
#include <iostream>
#include <fstream>
#include <sstream>

// run time checking(/RTC) makes the Microsoft implementation of std containers 
// *very* slow in debug builds !
// So in visual studio go to Project -> Properties -> C/C++ -> Code Generation -> Basic Runtime Checks and set to "Default"
#include <map>
#define GLM_FORCE_XYZW_ONLY
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "jsoncpp/json/json.h"

#include <xmmintrin.h>
#include <stdlib.h>
#include <random>

#define ANGLE_TORADIANS_COF glm::pi<float>() / 180.0f
#define RADIANS_TOANGLE_COF 180.0f / glm::pi<float>()

#ifdef FE_DEBUG_ENABLED
	#define FE_GL_ERROR(glCall)            \
	{                                      \
		glCall;                            \
		GLenum error = glGetError();	   \
		if (error != 0)                    \
		{								   \
			assert("FE_GL_ERROR" && 0);	   \
		}                                  \
	}
#else
	#define FE_GL_ERROR(glCall)            \
	{                                      \
		glCall;                            \
	}
#endif // FE_GL_ERROR

#define FE_MAX_LIGHTS 10
#define FE_CSM_UNIT 28
#define FE_WIN_32
#define FE_MESH_VERSION 0.02f
#define FE_TEXTURE_VERSION 0.02f

#define FE_SIMD_ENABLED

//#define FE_OLD_LOAD
//#define FE_OLD_SAVE

//#define FE_GPUMEM_ALLOCATION_LOGING

#define USE_OCCLUSION_CULLING