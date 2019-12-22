#pragma once

#include "glew2/include/GL/glew.h"
#include "glew2/include/GL/wglew.h"

#include <GLFW/glfw3.h>
#include <GL/GL.h>
#include <iostream>

#define GLM_FORCE_XYZW_ONLY

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.inl"

#define ANGLE_TORADIANS_COF glm::pi<float>() / 180.0f

#define SINGLETON_COPY_ASSIGN_PART(CLASS_NAME) \
CLASS_NAME(const CLASS_NAME &);                \
void operator= (const CLASS_NAME &);