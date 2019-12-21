#pragma once

#include "../ThirdParty/glew2/include/GL/glew.h"
#include <GL/GL.h>
#include <vector>

GLuint png_texture_load(const char * file_name);
std::vector<unsigned char> png_texture_loadData(const char* file_name, int& width, int& height);
