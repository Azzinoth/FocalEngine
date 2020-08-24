#pragma once

#include "FETime.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "GL/glew.h"
#include "GL/wglew.h"

#include <GLFW/glfw3.h>
#include <GL/GL.h>

#include <iostream>
#include <fstream>
#include <sstream>

// run time checking(/RTC) makes the Microsoft implementation of std containers 
// *very* slow in debug builds !
// So in visual studio go to Project -> Properties -> C/C++ -> Code Generation -> Basic Runtime Checks and set to "Default"
#include <unordered_map>
#include <map>
#define GLM_FORCE_XYZW_ONLY
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/quaternion.hpp"

#include "jsoncpp/json/json.h"

#define ANGLE_TORADIANS_COF glm::pi<float>() / 180.0f

#define SINGLETON_PUBLIC_PART(CLASS_NAME)  \
static CLASS_NAME& getInstance()           \
{										   \
	if (!_instance)                        \
		_instance = new CLASS_NAME();      \
	return *_instance;				       \
}                                          \
										   \
~CLASS_NAME();

#define SINGLETON_PRIVATE_PART(CLASS_NAME) \
static CLASS_NAME* _instance;              \
CLASS_NAME();                              \
CLASS_NAME(const CLASS_NAME &);            \
void operator= (const CLASS_NAME &);

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

#define FE_MAP_TO_STR_VECTOR(map)          \
std::vector<std::string> result;           \
auto iterator = map.begin();               \
while (iterator != map.end())              \
{                                          \
	result.push_back(iterator->first);     \
	iterator++;                            \
}                                          \
                                           \
return result;

#define FE_MAX_LIGHTS 10
#define FE_CSM_UNIT 16
#define FE_WIN_32