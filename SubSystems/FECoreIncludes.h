#pragma once

#include "glew2/include/GL/glew.h"
#include "glew2/include/GL/wglew.h"

#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define GLM_FORCE_XYZW_ONLY

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"

#define ANGLE_TORADIANS_COF glm::pi<float>() / 180.0f

#define SINGLETON_PUBLIC_PART(CLASS_NAME) \
static CLASS_NAME& getInstance()          \
{										  \
	if (!_instance)                       \
		_instance = new CLASS_NAME();     \
	return *_instance;				      \
}                                         \
										  \
~CLASS_NAME();

#define SINGLETON_PRIVATE_PART(CLASS_NAME) \
static CLASS_NAME* _instance;              \
CLASS_NAME();                              \
CLASS_NAME(const CLASS_NAME &);            \
void operator= (const CLASS_NAME &);

#define FE_DEBUG_ENABLED